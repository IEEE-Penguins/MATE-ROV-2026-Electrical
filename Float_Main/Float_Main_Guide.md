# Float_Main Usage and GUI Integration Guide

## 1. Purpose
This guide explains how to operate the Float_Main stack during mission runs, how the FSM works, and how to integrate the telemetry with your shore GUI in a reliable and judge-friendly way.

The Float_Main system is designed as an autonomous float controller:
- it controls buoyancy using the stepper-driven ballast mechanism,
- it tracks mission phases with a formal state machine,
- it transmits depth packets every 5 seconds,
- it retries and persists data when communication is unstable.

## 2. Module Overview
The Float_Main folder is modular. Each module has one clear responsibility.

1. Float_Main.ino
Orchestrator. Initializes modules, runs loop scheduling, triggers sampling, and prints runtime status.

2. FloatConfig.h
Single source of truth for pins, timing, depth limits, communication constants, queue sizes, and mission limits.

3. FloatTypes.h
Defines MissionPacketContext, which carries mission-state metadata into each outgoing telemetry packet.

4. MissionController.h and MissionController.cpp
Implements FSM and mission scoring logic.

5. CommsLink.h and CommsLink.cpp
Handles Wi-Fi connection, HTTP transmission, queueing, retry backoff, and LittleFS persistence.

6. Depth.h and Depth.cpp
Sensor driver used to read pressure/depth from HX711-style interface and convert to meters.

7. Stepper.h and Stepper.cpp
Actuator driver for ballast movement, including step timing and position tracking.

## 3. Current Mission Constants (Important)
These values directly drive mission behavior:

1. Packet interval: 5000 ms
2. Control loop cadence in FSM: 40 ms
3. Stepper correction cadence in FSM: 30 ms
4. Hold duration per target band: 30000 ms
5. Required hold evidence samples: 7
6. Mission max duration: 15 minutes
7. Target successful profiles: 2
8. Maximum profile attempts: 6

Depth windows currently configured:

1. Deep band at sensor: 2.27 m to 2.83 m
2. Sensor-to-top offset: 0.65 m
3. Top shallow target band: 0.07 m to 0.73 m
4. Shallow band at sensor after offset conversion: 0.72 m to 1.38 m

Penalty threshold currently configured:

1. Surface contact threshold at sensor: depth <= 0.65 m

## 4. How to Use Float_Main in Mission
## 4.1 Pre-Mission Setup
1. Set network and company values in FloatConfig.h:
- WIFI_SSID
- WIFI_PASS
- COMPANY_ID
- SERVER_BASE_URL

2. Verify physical pins in FloatConfig.h:
- stepper DIR and STEP pins
- depth DOUT and SCK pins

3. Verify direction convention:
- STEPPER_DIR_FOR_DESCEND must match physical hardware behavior.

4. Power up and open serial monitor at 115200.

5. Confirm startup logs:
- depth calibration completed
- Wi-Fi connection attempts
- health check results when connected

## 4.2 In-Mission Runtime
1. Float_Main.ino reads depth continuously.
2. CommsLink runs continuously for reconnect, retry, and queue flush.
3. MissionController runs continuously for phase transitions and control actions.
4. Every 5 seconds:
- hold evidence sampling is updated,
- one packet is transmitted or queued,
- first successful transmit gate can unlock descent.

5. Every 1 second:
- status line is printed for operator awareness.

## 4.3 Post-Mission
1. Confirm queuedCount returned to zero after reconnect.
2. Export received packets from shore server.
3. Generate depth-vs-time graph for judging.

## 5. FSM Detailed Explanation
MissionController uses these states:

1. kAwaitFirstTransmit
2. kDescendToDeep
3. kHoldDeep
4. kAscendToShallow
5. kHoldShallow
6. kProfileComplete
7. kRecoveryReady

State behavior:

1. kAwaitFirstTransmit
- goal: send at least one packet before starting profile descent,
- control: keeps float around shallow zone while waiting,
- exit: markFirstTransmitSuccess() then transition to kDescendToDeep.

2. kDescendToDeep
- goal: move sensor depth into deep band 2.27 to 2.83 m,
- control: stepper moves deeper or shallower by micro-corrections,
- exit: transition to kHoldDeep when depth enters range.

3. kHoldDeep
- goal: maintain deep band for full 30 seconds with evidence packets,
- evidence rule: holdSampleCount grows on sample ticks,
- reset rule: if depth leaves deep band, hold timer and samples reset,
- exit: after 30 seconds and 7 samples, transition to kAscendToShallow.

4. kAscendToShallow
- goal: move to shallow sensor band 0.72 to 1.38 m,
- control: same correction pattern,
- exit: transition to kHoldShallow when in range.

5. kHoldShallow
- goal: maintain shallow band for 30 seconds with evidence packets,
- reset rule: leaving band resets timer and samples,
- exit: after timer and sample criteria, transition to kProfileComplete.

6. kProfileComplete
- goal: score this profile,
- scoring:
- if no penalty, successfulProfiles increments,
- completedProfiles always increments,
- if successfulProfiles reaches target or attempts reach max, go to recovery,
- else start next profile at kDescendToDeep.

7. kRecoveryReady
- goal: maintain safe shallow control and end active profiling,
- entry conditions:
- two successful profiles reached,
- or max attempts reached,
- or mission time limit reached.

Penalty handling:
- During active profile states, if depth <= SURFACE_CONTACT_SENSOR_DEPTH_M, profilePenalty becomes true for that profile.

Mission time limit handling:
- If elapsed mission time >= 15 minutes, controller forces kRecoveryReady.

## 6. Functionality of Each Module
## 6.1 Float_Main.ino
Main responsibilities:

1. Initialize modules in setup()
2. Read and sanitize depth
3. Call commsLink.update(nowMs)
4. Mirror first successful transmission to MissionController
5. Call missionController.update(nowMs, depth)
6. Run 5-second sample pipeline
7. Print operator status every 1 second

Design pattern:
- thin orchestrator,
- business logic delegated to MissionController and CommsLink.

## 6.2 MissionController
Core responsibilities:

1. Implement state machine transitions
2. Perform depth-range control via stepper micro-step commands
3. Track hold timers and hold sample counts
4. Track profile completion, penalties, success counts
5. Build packet context for telemetry

Control strategy:
- uses a deadband around min and max range,
- uses center trim logic for finer correction,
- enforces stepping rate limit with STEPPER_STEP_INTERVAL_MS.

## 6.3 CommsLink
Core responsibilities:

1. Build packet payload from depth and mission context
2. Send live packet to /api/float/packet
3. Queue packet when send fails
4. Retry queued packets to /api/float/packets
5. Persist queue in LittleFS for recovery across reboot
6. Apply exponential backoff from 1 second to 30 seconds
7. Attempt Wi-Fi reconnect every 10 seconds

Reliability model:
- online path: immediate send,
- offline path: queue + persist,
- recovery path: batch flush on reconnect.

## 6.4 Depth Module
Core responsibilities:

1. Read raw 24-bit sensor value with sign extension
2. Calibrate zero offset at startup
3. Average N samples per read
4. Convert to pressure and depth

Safety additions in current code:

1. waitReady timeout to avoid indefinite blocking
2. sample guards when sample count is zero
3. fallback to last value if sensor not ready in time

## 6.5 Stepper Module
Core responsibilities:

1. Configure motor pins
2. Set speed in steps per second
3. Move in absolute angle or relative steps
4. Execute single-direction step for control-loop micro-adjustments
5. Track current position in steps

Safety additions in current code:

1. guard invalid speed values
2. guard invalid steps-per-revolution values
3. minimum pulse delay enforcement

## 6.6 FloatConfig and FloatTypes
FloatConfig:
- all tunable constants are centralized here,
- no magic numbers should be scattered in modules.

FloatTypes:
- MissionPacketContext is a compact transport object from FSM to telemetry module.

## 7. Packet Schema Sent to Shore
Each packet includes required mission fields and extra context.

Required scoring fields:

1. companyId
2. timestamp (mission elapsed time)
3. pressureKpa
4. depthMeters

Reliability and traceability fields:

1. packetId
2. seq

FSM context fields:

1. profileIndex
2. phase
3. inDeepRange
4. inShallowRange
5. profilePenalty
6. recoveryReady
7. successfulProfiles
8. completedProfiles
9. holdSampleCount

## 8. Best Approach to Integrate with Your GUI
Recommended architecture:

1. Float firmware posts packets to mission server (already implemented).
2. GUI reads data from mission server database or stream layer.
3. GUI does not control tight closed-loop depth behavior in real time.
4. GUI focuses on visibility, evidence validation, and operator decisions.

Why this is best:

1. Keeps control loop deterministic on-device.
2. Avoids network latency affecting buoyancy control.
3. Preserves mission continuity during temporary shore network issues.

## 8.1 GUI Screens to Implement
1. Live Mission Dashboard
- current phase
- current profile number
- live depth
- in-range indicators
- queue depth
- link health

2. Hold Evidence Panel
- deep hold timer and sample count
- shallow hold timer and sample count
- automatic pass or fail indicator for 30-second hold requirement

3. Packet Monitor
- latest packet JSON
- sequence continuity check
- duplicate packetId detection

4. Judge Mode View
- highlight sequential packets at 0,5,10,15,20,25,30 seconds
- one-click export of packets used as evidence

5. Post-Mission Graph
- depth on Y axis,
- time on X axis,
- profile segmentation markers,
- penalty markers if surface threshold crossed.

## 8.2 GUI Data Pipeline Recommendation
1. Backend ingest endpoints must match firmware:
- POST /api/float/packet
- POST /api/float/packets
- GET /health

2. Store packetId as unique key to support idempotent inserts.
3. Build a mission session id from companyId plus bootNonce part of packetId.
4. Stream processed telemetry to GUI via WebSocket or SSE.
5. Keep raw packet history for audit and judge review.

## 8.3 GUI Validation Rules
Implement these rules in GUI backend for automatic checks:

1. Deep pass window is [2.27, 2.83] meters.
2. Shallow sensor pass window is [0.72, 1.38] meters.
3. Hold pass requires:
- continuous in-range behavior,
- duration >= 30 seconds,
- at least 7 sequential 5-second samples.

4. Penalty flag if depth <= 0.65 meters during active profile.

## 9. Suggested Mission-Day Workflow with GUI
1. Start mission server and GUI.
2. Verify /health endpoint from ESP32 network.
3. Power on float and confirm first packet appears.
4. Confirm FSM moves from AwaitFirstTransmit to DescendToDeep.
5. Monitor deep hold evidence, then shallow hold evidence.
6. Monitor profile completion and success counters.
7. If penalties occur, allow additional profiles until best two successes are achieved.
8. After recovery, export packet set and graph for judge presentation.

## 10. Practical Tuning Strategy
Tune in this order:

1. Stepper direction and polarity first.
2. Stepper speed and step interval second.
3. Depth calibration factor and sensor offset third.
4. Range margin and center trim margin last.

Use one change at a time and compare GUI graph outputs between runs.

## 11. Known Operational Notes
1. CommsLink begin() returns filesystem readiness state; mission still runs even if flash persistence is unavailable.
2. If queue grows continuously during mission, communication path is degraded. Preserve packets and allow resend after reconnect.
3. Hold logic resets immediately on out-of-range readings. This is strict by design.

## 12. Quick Checklist
1. Config values verified
2. Pins verified
3. Stepper direction verified
4. Depth calibration performed
5. Server endpoints live
6. GUI receiving packets
7. First transmit confirmed before descent
8. Evidence packets for both depth holds confirmed

This is the recommended professional workflow for using Float_Main with your mission GUI reliably and transparently.