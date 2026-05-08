# Float_Main Main Sketch Explanation

## Scope
This document explains the orchestration logic in Float_Main.ino in a production-style, system-level way.

The sketch is intentionally thin. It does not embed mission state logic or networking internals directly. Instead, it coordinates specialized modules and enforces execution timing.

## Architectural Role of Float_Main.ino
Float_Main.ino is the runtime coordinator for four subsystems:

1. Depth sensing (Depth)
2. Buoyancy actuation (Stepper)
3. Mission profile state machine (MissionController)
4. Communication and reliability pipeline (CommsLink)

This separation keeps responsibilities clean:

- Float_Main.ino handles scheduling and wiring.
- MissionController decides what the float should do next.
- CommsLink decides how telemetry gets delivered reliably.

## Included Modules and Why They Matter

- CommsLink.h: Wi-Fi, HTTP post, queueing, retries, persistence.
- Depth.h: pressure/depth sensor interface.
- FloatConfig.h: all mission constants (pins, rates, ranges, limits).
- MissionController.h: explicit mission phase logic.
- Stepper.h: buoyancy engine actuator driver.

## Global Objects and Runtime Ownership

The sketch creates one global instance per subsystem:

- depthSensor
- ballastStepper
- missionController(ballastStepper)
- commsLink

This is important because all of them are long-lived services with internal state that must persist for the entire mission (timers, queues, counters, profile progress).

Two scheduler timestamps are also global:

- lastSampleAt
- lastStatusPrintAt

They implement non-blocking periodic tasks based on millis().

## Helper Function 1: readDepthMeters()

Purpose:
- Read current depth from depthSensor.
- Sanitize value before mission and communication layers consume it.

Behavior:
- Calls depthSensor.getDepthMeters().
- Clamps negative values to 0.0.
- Clamps outlier values to FloatConfig::MAX_VALID_SENSOR_DEPTH_M.

Why this matters:
- Protects mission logic from transient sensor glitches.
- Prevents impossible values from being transmitted and stored.

## Helper Function 2: printRuntimeStatus(nowMs, depthMeters)

Purpose:
- Emit operator-facing runtime telemetry every STATUS_PRINT_INTERVAL_MS.

What it prints:
- Mission phase name
- Profile index
- Completed and successful profile counters
- Penalty flag
- Current depth
- In-range flags (deep and shallow)
- Hold sample count
- Hold elapsed time
- Queue size
- Whether any successful transmission occurred

Why this matters:
- Supports field debugging and judge-facing traceability.
- Gives immediate visibility into mission progression and communication health.

## setup() Execution Breakdown

setup() performs deterministic boot sequencing:

1. Start serial link at 115200 and print startup banner.
2. Initialize depth sensor pins from FloatConfig.
3. Apply depth calibration factor and averaging sample count.
4. Run startup depth calibration using DEPTH_CALIBRATE_SAMPLES.
5. Initialize stepper pins and movement speed.
6. Start MissionController with current mission time baseline.
7. Start CommsLink (Wi-Fi/reliability stack initialization).
8. Prime schedulers:
   - lastSampleAt is set so first sample can be sent immediately.
   - lastStatusPrintAt is initialized to now.

Design note:
Immediate sample eligibility at boot reduces delay before first packet visibility.

## loop() Execution Breakdown

The loop has a fixed order each cycle, which is critical for determinism.

### 1) Acquire current time and depth
- nowMs = millis()
- depthMeters = readDepthMeters()

This creates a coherent snapshot for this loop iteration.

### 2) Advance communication subsystem
- commsLink.update(nowMs)

This lets networking progress independently of sample tick boundaries (reconnects, retries, queue flushes, persistence timing).

### 3) Enforce first-successful-transmit mission gate
If comms already has a successful transmit and mission has not been notified yet:
- missionController.markFirstTransmitSuccess()

This aligns with competition requirements to transmit before descent profile execution.

### 4) Advance mission state machine
- missionController.update(nowMs, depthMeters)

This drives phase transitions and stepper correction decisions at the control cadence configured in FloatConfig.

### 5) Run periodic 5-second sample pipeline
When SAMPLE_INTERVAL_MS elapses:

1. missionController.onSampleTick(nowMs, depthMeters)
2. commsLink.sendSample(depthMeters, missionController.packetContext(), nowMs)
3. If current send succeeds and first-transmit gate was still closed, mark first transmit success.

Important detail:
The sample packet includes mission context from MissionController (phase, profile, range flags, etc.), not only raw depth.

### 6) Run periodic status print
When STATUS_PRINT_INTERVAL_MS elapses:
- printRuntimeStatus(nowMs, depthMeters)

### 7) Small loop pacing delay
- delay(10)

This prevents unnecessary busy-spinning while preserving responsive updates.

## Timing Model in Practice

The effective runtime cadences are:

- High-frequency loop: approximately every 10-20 ms (depends on workload).
- Mission control updates: MissionController internally gates by CONTROL_INTERVAL_MS.
- Stepper micro-adjust pacing: MissionController internally gates by STEPPER_STEP_INTERVAL_MS.
- Data packet generation/transmit: every SAMPLE_INTERVAL_MS (5 seconds).
- Human-readable status output: every STATUS_PRINT_INTERVAL_MS (1 second).

This mixed-rate design is intentional:
- Fast enough control for depth behavior.
- Fixed 5-second evidence packets for scoring requirements.
- Independent comms maintenance in background.

## Data and Control Flow Summary

1. Sensor path:
   Depth sensor -> readDepthMeters() -> sanitized depth value.

2. Mission path:
   sanitized depth + nowMs -> MissionController.update() -> phase/control decisions.

3. Telemetry path:
   sanitized depth + mission packet context -> CommsLink.sendSample() ->
   immediate HTTP send or queued retry/persisted delivery.

4. Gate coupling:
   Successful transmission status from CommsLink -> first-transmit gate in MissionController.

## Why This Main Sketch Design Is Professional and Maintainable

- Single responsibility at orchestration level.
- Clear fixed-order execution flow in loop().
- Time-based scheduling instead of blocking task chains.
- Mission and communication concerns are isolated behind interfaces.
- Runtime observability is built in.

## Operational Notes for Team Usage

1. Validate depth calibration at startup in deployment conditions.
2. Confirm stepper direction constant (STEPPER_DIR_FOR_DESCEND) matches physical mechanism.
3. Keep SAMPLE_INTERVAL_MS at or below competition packet cadence requirement.
4. Watch queue size in status output to detect prolonged connectivity issues.
5. Use phase and hold counters in status output to verify profile progress in real time.

## Suggested Future Enhancements

1. Add serial command interface for start/stop/recalibration.
2. Add persistent mission event log for post-run evidence extraction.
3. Add watchdog/reset strategy for long communication outages.
4. Add low-pass filtering on depth before control updates if sensor noise is high.
5. Add an explicit simulation mode to replay synthetic depth traces for dry testing.
