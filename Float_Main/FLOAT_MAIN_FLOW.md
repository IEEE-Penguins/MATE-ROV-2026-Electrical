# Float_Main.ino Code Flow and Function Usage

This document explains the full runtime flow of `Float_Main.ino` and every function used by the main sketch.

## 1. File Role

`Float_Main.ino` is the orchestrator. It does not implement low-level sensor, stepper, mission, or network internals. Instead, it wires modules together and drives them on fixed timing intervals.

Main modules used:
- `Depth` (`depthSensor`): reads depth from sensor.
- `Stepper` (`ballastStepper`): moves ballast mechanism.
- `MissionController` (`missionController`): mission state machine and movement decisions.
- `CommsLink` (`commsLink`): telemetry send, queue, retry, and persistence.

## 2. Global Runtime State in the Sketch

- `Depth depthSensor;`
- `Stepper ballastStepper;`
- `MissionController missionController(ballastStepper);`
- `CommsLink commsLink;`
- `uint32_t lastSampleAt = 0;`
- `uint32_t lastStatusPrintAt = 0;`

Scheduling variables:
- `lastSampleAt`: controls telemetry sample period (`SAMPLE_INTERVAL_MS`).
- `lastStatusPrintAt`: controls status print period (`STATUS_PRINT_INTERVAL_MS`).

## 3. Functions Defined in Float_Main.ino

### `readDepthMeters()`
- Reads raw depth from `depthSensor.getDepthMeters()`.
- Clamps negative values to `0.0f`.
- Clamps very large outliers to `FloatConfig::MAX_VALID_SENSOR_DEPTH_M`.
- Returns the sanitized depth used by both mission logic and telemetry.

### `printRuntimeStatus(nowMs, depthMeters)`
- Pulls mission metadata from `missionController.packetContext()`.
- Prints a single status line containing:
  - Current mission phase (`missionController.phaseName()`)
  - Profile counters
  - Penalty flag
  - Current depth
  - Deep/shallow in-range flags
  - Hold sample count
  - Hold elapsed ms (`missionController.currentHoldElapsedMs(nowMs)`)
  - Queue size (`commsLink.queuedCount()`)
  - Any successful transmit flag (`commsLink.hasSuccessfulTransmit()`)

### `setup()`
Boot sequence:
1. Starts serial (`Serial.begin(115200)`) and prints startup banner.
2. Initializes depth sensor pins using `depthSensor.begin(...)`.
3. Applies sensor tuning:
   - `depthSensor.setCalibrationFactor(...)`
   - `depthSensor.setSamples(...)`
4. Performs startup calibration: `depthSensor.calibrate(...)`.
5. Initializes stepper:
   - `ballastStepper.begin(...)`
   - `ballastStepper.setSpeed(...)`
6. Starts mission state machine: `missionController.begin(millis())`.
7. Starts comms subsystem: `commsLink.begin()`.
8. Initializes scheduler timestamps.

### `loop()`
Loop cycle order:
1. Acquire current time and depth.
2. Call `commsLink.update(nowMs)` to progress Wi-Fi/retry/queue tasks.
3. If comms has a successful send and mission gate is still closed:
   - `missionController.markFirstTransmitSuccess()`
4. Run mission control step:
   - `missionController.update(nowMs, depthMeters)`
5. On sample interval:
   - `missionController.onSampleTick(nowMs, depthMeters)`
   - `commsLink.sendSample(depthMeters, missionController.packetContext(), nowMs)`
   - If sent now and first-transmit gate still closed, mark first transmit success.
6. On status print interval:
   - `printRuntimeStatus(nowMs, depthMeters)`
7. `delay(10)` to avoid tight busy loop.

## 4. Function Call Graph from Main Sketch

### Called from `setup()`
- `Serial.begin`
- `delay`
- `Serial.println`
- `depthSensor.begin`
- `depthSensor.setCalibrationFactor`
- `depthSensor.setSamples`
- `depthSensor.calibrate`
- `ballastStepper.begin`
- `ballastStepper.setSpeed`
- `missionController.begin`
- `commsLink.begin`
- `millis`

### Called from `loop()`
- `millis`
- `readDepthMeters`
- `commsLink.update`
- `commsLink.hasSuccessfulTransmit`
- `missionController.hasFirstTransmit`
- `missionController.markFirstTransmitSuccess`
- `missionController.update`
- `missionController.onSampleTick`
- `missionController.packetContext`
- `commsLink.sendSample`
- `printRuntimeStatus`
- `delay`

### Called inside `readDepthMeters()`
- `depthSensor.getDepthMeters`

### Called inside `printRuntimeStatus()`
- `missionController.packetContext`
- `missionController.phaseName`
- `missionController.currentHoldElapsedMs`
- `commsLink.queuedCount`
- `commsLink.hasSuccessfulTransmit`
- `Serial.printf`

## 5. Timing and Behavior Model

Main timing layers:
- Loop pacing: roughly every 10-20 ms (depends on work and delays).
- Mission control cadence: inside `MissionController::update`, gated by `CONTROL_INTERVAL_MS`.
- Telemetry sample cadence: every `SAMPLE_INTERVAL_MS`.
- Status print cadence: every `STATUS_PRINT_INTERVAL_MS`.

Why this design works:
- Comms maintenance runs continuously, independent of sample boundaries.
- Mission control decisions run frequently enough for smooth depth correction.
- Sample transmission stays fixed to competition-friendly intervals.

## 6. First-Transmit Mission Gate

Descent mission logic should not fully proceed before at least one successful transmit is observed.

The sketch enforces this in two places:
- Immediately after `commsLink.update(...)`.
- Immediately after periodic `sendSample(...)` succeeds.

Both paths call `missionController.markFirstTransmitSuccess()` only if needed.

## 7. Data Flow Across Modules

1. Sensor path:
   `Depth::getDepthMeters` -> `readDepthMeters` clamp/sanitize -> mission + comms consumers.

2. Mission path:
   `missionController.update` consumes depth/time -> controls phase and stepper behavior internally.

3. Telemetry path:
   `missionController.packetContext` + depth + time -> `commsLink.sendSample`.

4. Reliability path:
   `commsLink.update` handles reconnect, queue retry, backoff, and periodic queue persistence.

## 8. Practical Debug Checklist

- If mission does not start descending, verify first successful transmit is happening.
- If queue keeps growing, check Wi-Fi and server health endpoint.
- If hold phases never complete, verify depth ranges and sample interval configuration.
- If depth oscillates, tune control margins and stepper speed/interval.
