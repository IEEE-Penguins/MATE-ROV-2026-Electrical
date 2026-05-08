# Float Main Function Reference

This document explains every function found in this directory.

## Project-Level Notes

- `FloatConfig.h` defines constants only (no functions).
- `FloatTypes.h` defines `MissionPacketContext` only (no functions).
- `explanation.md` is narrative documentation (no functions).
- `.vscode/c_cpp_properties.json`, `.vscode/launch.json`, and `.vscode/settings.json` are editor/runtime configuration files (no functions).

## Float_Main.ino

### `float readDepthMeters()`
- Purpose: Reads the current sensor depth and clamps it to a safe range.
- Inputs: None.
- Returns: Depth in meters in `[0, FloatConfig::MAX_VALID_SENSOR_DEPTH_M]`.
- Side effects: Calls `depthSensor.getDepthMeters()`.

### `void printRuntimeStatus(uint32_t nowMs, float depthMeters)`
- Purpose: Prints current mission and communication status to Serial.
- Inputs:
  - `nowMs`: Current time from `millis()`.
  - `depthMeters`: Current depth value already sanitized by `readDepthMeters()`.
- Returns: Nothing.
- Side effects: Reads mission context and communication state, writes formatted status text via `Serial.printf`.

### `void setup()`
- Purpose: Boot-time initialization for serial, sensor, stepper, mission state machine, and comms.
- Inputs: None.
- Returns: Nothing.
- Side effects:
  - Initializes Serial.
  - Configures and calibrates depth sensor.
  - Configures stepper and speed.
  - Starts mission controller and comms link.
  - Initializes periodic scheduler timestamps.

### `void loop()`
- Purpose: Main runtime scheduler for depth read, mission update, sampling, communication, and status output.
- Inputs: None.
- Returns: Nothing.
- Side effects:
  - Updates comms and mission state continuously.
  - Sends periodic packets.
  - Prints periodic status.
  - Delays 10 ms each cycle.

## CommsLink.cpp / CommsLink.h

### `float computePressureKpa(float depthMeters)` (file-local helper)
- Purpose: Converts depth meters to pressure in kPa.
- Inputs: `depthMeters`.
- Returns: `depthMeters * FloatConfig::PRESSURE_KPA_PER_METER` (after clamping negative depth to zero).

### `bool CommsLink::begin()`
- Purpose: Initializes filesystem, URLs, random nonce, optional queue restore, and Wi-Fi connection attempt.
- Returns: `true` only if LittleFS initialized successfully.
- Side effects: May load persisted queue and start Wi-Fi connection.

### `void CommsLink::update(uint32_t nowMs)`
- Purpose: Runs periodic comms maintenance tasks.
- Tasks:
  - Print Wi-Fi status changes.
  - Reconnect Wi-Fi when needed.
  - Health-check server once per connected session.
  - Flush queued packets when retry timer allows.
  - Persist queue periodically.

### `bool CommsLink::sendSample(float depthMeters, const MissionPacketContext& context, uint32_t nowMs)`
- Purpose: Build and transmit one packet, or queue it if transmission fails/offline.
- Returns: `true` if sent immediately, otherwise `false`.
- Side effects: May enqueue, persist queue, and update retry backoff.

### `bool CommsLink::hasSuccessfulTransmit() const`
- Purpose: Reports whether any packet has ever been successfully transmitted since boot.

### `size_t CommsLink::queuedCount() const`
- Purpose: Returns number of packets currently in retry queue.

### `size_t CommsLink::queueIndex(size_t offsetFromHead) const`
- Purpose: Circular-buffer index helper.
- Returns: Wrapped array index for queue access.

### `bool CommsLink::queueIsFull() const`
- Purpose: Checks if queue reached `FloatConfig::MAX_QUEUE_SIZE`.

### `bool CommsLink::queueIsEmpty() const`
- Purpose: Checks if queue has zero packets.

### `bool CommsLink::queueContainsPacketId(const char* packetId) const`
- Purpose: Dedup helper to avoid storing duplicate packets.

### `bool CommsLink::enqueuePacket(const FloatPacket& packet)`
- Purpose: Adds packet to queue (drops oldest when full).
- Returns: `true` (including duplicate/no-op path).

### `bool CommsLink::dequeuePackets(size_t n)`
- Purpose: Removes `n` oldest packets from queue.
- Returns: `false` if `n > queueCount_`, else `true`.

### `String CommsLink::missionTimestamp(uint32_t nowMs) const`
- Purpose: Converts uptime milliseconds to `H:MM:SS` mission timestamp string.

### `CommsLink::FloatPacket CommsLink::buildPacket(float depthMeters, const MissionPacketContext& context, uint32_t nowMs)`
- Purpose: Creates telemetry packet with mission metadata and unique packet id.
- Side effects: Increments sequence counter.

### `void CommsLink::packetToJson(JsonObject obj, const FloatPacket& packet) const`
- Purpose: Serializes packet struct into JSON object.

### `bool CommsLink::jsonToPacket(JsonObject obj, FloatPacket& out) const`
- Purpose: Parses one JSON packet entry into `FloatPacket`.
- Returns: `false` if required depth field is missing/invalid.

### `bool CommsLink::persistQueueToFlash()`
- Purpose: Writes in-memory retry queue to `FloatConfig::QUEUE_FILE` in LittleFS.
- Returns: `true` on successful write and serialization.

### `bool CommsLink::loadQueueFromFlash()`
- Purpose: Restores retry queue from LittleFS at startup.
- Returns: `true` for successful restore (or empty/missing queue file), `false` on read/parse failures.
- Side effects: Rebuilds queue and updates sequence counter to avoid id reuse.

### `void CommsLink::setRetryBackoffFailure(uint32_t nowMs)`
- Purpose: Schedules next retry and doubles retry delay with cap.

### `void CommsLink::resetRetryBackoffSuccess()`
- Purpose: Resets retry delay and clears next retry deadline after success.

### `bool CommsLink::is2xx(int code) const`
- Purpose: Utility that checks if HTTP status code is in `[200, 299]`.

### `bool CommsLink::postJson(const String& url, const String& jsonBody, int& httpCode, String& responseBody)`
- Purpose: Shared HTTP POST helper with timeout and JSON content type.
- Returns: `true` on 2xx response.
- Side effects: Populates `httpCode` and `responseBody`.

### `bool CommsLink::sendSinglePacket(const FloatPacket& packet)`
- Purpose: Sends one packet to `/api/float/packet`.
- Returns: `true` on 2xx.
- Side effects: Serial logging of success/failure.

### `bool CommsLink::sendBatchFromQueue(size_t batchSize)`
- Purpose: Sends oldest queued packets to `/api/float/packets`.
- Returns: `true` on 2xx batch transmit.
- Side effects: Dequeues and persists queue on success.

### `void CommsLink::connectWiFiIfNeeded(uint32_t nowMs)`
- Purpose: Attempts Wi-Fi reconnect no faster than configured retry interval.

### `void CommsLink::printNetworkStatusIfChanged()`
- Purpose: Emits Wi-Fi status changes and connected IP once status changes.

### `bool CommsLink::checkServerHealth()`
- Purpose: Performs GET on `/health` endpoint.
- Returns: `true` on 2xx response.

### `void CommsLink::flushRetryQueueIfDue(uint32_t nowMs)`
- Purpose: Tries to send queued packets when connected and retry timer allows.
- Side effects: Updates success flag/backoff state.

### `void CommsLink::maybePersistQueuePeriodic(uint32_t nowMs)`
- Purpose: Periodically snapshots queue to flash based on `PERSIST_INTERVAL_MS`.

## Depth.cpp / Depth.h

### `bool Depth::waitReady(uint32_t timeoutUs)`
- Purpose: Waits for HX711-like data-ready signal with timeout.
- Returns: `true` when ready, `false` on timeout.

### `long Depth::readRaw()`
- Purpose: Reads one 24-bit raw ADC sample and sign-extends it.
- Returns: Raw sample, or previous cached value if sensor not ready in time.

### `void Depth::begin(uint8_t dout_pin, uint8_t sck_pin)`
- Purpose: Configures sensor GPIO pins.

### `void Depth::calibrate(uint8_t samples)`
- Purpose: Captures baseline offset from averaged raw samples.
- Behavior: Forces minimum sample count of 1.

### `void Depth::setCalibrationFactor(float factor)`
- Purpose: Sets conversion factor from raw counts to pressure-like units.

### `void Depth::setSamples(uint8_t samples)`
- Purpose: Sets averaging count for `read()`.
- Behavior: Forces minimum sample count of 1.

### `long Depth::read()`
- Purpose: Returns averaged raw sample over configured sample count.

### `float Depth::getPressure()`
- Purpose: Converts offset-corrected raw reading to pressure using calibration factor.

### `float Depth::getDepthMeters()`
- Purpose: Converts pressure to depth in meters using 9810 divisor.

### `float Depth::getDepthCM()`
- Purpose: Converts depth meters to centimeters.

## MissionController.cpp / MissionController.h

### `const char* phaseToName(MissionPhase phase)` (file-local helper)
- Purpose: Maps enum phase to human-readable string.

### `MissionController::MissionController(Stepper& stepper)`
- Purpose: Injects stepper reference and initializes controller state fields.

### `void MissionController::begin(uint32_t nowMs)`
- Purpose: Resets mission state to start-of-run defaults.

### `void MissionController::markFirstTransmitSuccess()`
- Purpose: Opens mission gate after first successful packet transmit.
- Side effects: If waiting gate is active, transitions to deep descent phase.

### `bool MissionController::hasFirstTransmit() const`
- Purpose: Returns whether first successful transmit gate has been satisfied.

### `void MissionController::update(uint32_t nowMs, float depthMeters)`
- Purpose: Main mission state-machine update.
- Responsibilities:
  - Enforce mission duration limit.
  - Apply control-rate gating.
  - Apply profile penalty logic.
  - Run phase-specific transition/control behavior.

### `void MissionController::onSampleTick(uint32_t nowMs, float depthMeters)`
- Purpose: Updates hold sample counting on fixed telemetry sample cadence.
- Behavior: Counts only when in correct hold phase and in valid depth range.

### `MissionPacketContext MissionController::packetContext() const`
- Purpose: Builds packet metadata snapshot for telemetry.

### `MissionPhase MissionController::phase() const`
- Purpose: Returns current mission phase enum.

### `const char* MissionController::phaseName() const`
- Purpose: Returns current phase name string.

### `uint8_t MissionController::currentProfile() const`
- Purpose: Returns active profile index (1-based in current logic).

### `uint8_t MissionController::successfulProfiles() const`
- Purpose: Returns number of successful profiles (no penalty).

### `uint8_t MissionController::completedProfiles() const`
- Purpose: Returns number of completed profile attempts.

### `bool MissionController::profilePenaltyActive() const`
- Purpose: Returns whether current profile is penalized.

### `bool MissionController::isRecoveryReady() const`
- Purpose: Returns true when controller is in recovery phase.

### `uint8_t MissionController::currentHoldSampleCount() const`
- Purpose: Returns number of valid hold samples collected in active hold phase.

### `uint32_t MissionController::currentHoldElapsedMs(uint32_t nowMs) const`
- Purpose: Returns elapsed hold duration in ms since hold tracking started.

### `bool MissionController::inDeepRange(float depthMeters) const`
- Purpose: Checks deep target range `[DEEP_MIN_M, DEEP_MAX_M]`.

### `bool MissionController::inShallowRange(float depthMeters) const`
- Purpose: Checks shallow target range `[SHALLOW_SENSOR_MIN_M, SHALLOW_SENSOR_MAX_M]`.

### `void MissionController::transitionTo(MissionPhase nextPhase, uint32_t nowMs)`
- Purpose: Changes mission phase and resets hold tracking for relevant phase groups.

### `void MissionController::resetHoldTracking()`
- Purpose: Clears hold start time, last hold sample time, and hold sample counter.

### `void MissionController::updatePenalty(float depthMeters)`
- Purpose: Activates profile penalty if depth indicates surface contact.

### `void MissionController::processHoldState(uint32_t nowMs, float depthMeters, bool deepHold)`
- Purpose: Shared hold-phase logic for deep and shallow holds.
- Behavior:
  - Keeps depth in range via control.
  - Resets hold if range lost.
  - Requires both hold duration and required sample count before transition.

### `void MissionController::finalizeProfile(uint32_t nowMs)`
- Purpose: Completes one profile attempt and decides next action.
- Decisions:
  - Increment completed count.
  - Increment successful count if not penalized.
  - End mission if success target or max attempts reached.
  - Otherwise start next profile.

### `void MissionController::controlTowardRange(uint32_t nowMs, float depthMeters, float minDepth, float maxDepth)`
- Purpose: Range-centered stepper correction logic with margin and trim behavior.
- Behavior:
  - Rate-limited by step interval.
  - Steps shallow/deep if outside margins.
  - Applies center-trim steps when inside range but not centered.

### `void MissionController::stepTowardDeeper(uint32_t nowMs)`
- Purpose: Commands one step in descend direction and updates step timestamp.

### `void MissionController::stepTowardShallower(uint32_t nowMs)`
- Purpose: Commands one step in opposite direction and updates step timestamp.

## Stepper.cpp / Stepper.h

### `uint32_t safeDelayMicros(float stepDelayUs)` (file-local helper)
- Purpose: Ensures step pulse delay is at least 1 microsecond.

### `void pulseStep(short stepPin, uint32_t delayUs)` (file-local helper)
- Purpose: Emits one full step pulse (HIGH then LOW) with symmetric delays.

### `void Stepper::begin(short dir_pin, short step_pin)`
- Purpose: Configures stepper direction and step output pins.

### `void Stepper::setSpeed(float stepsPerSecond)`
- Purpose: Converts desired step rate to microsecond half-period delay.
- Behavior: Enforces minimum practical values.

### `void Stepper::editStepsPerRevolution(float steps)`
- Purpose: Updates mechanical steps-per-revolution model and derived step angle.

### `void Stepper::moveToAngle(float angle)`
- Purpose: Moves motor to absolute target angle using current position model.
- Behavior:
  - Computes target step index by rounding `angle / stepAngle`.
  - Pulses required number of steps in proper direction.
  - Updates `currentPosition` to target steps.

### `void Stepper::moveSingleDirection(bool dir)`
- Purpose: Performs exactly one step in chosen direction and updates position.

### `int Stepper::getCurrentPosition()`
- Purpose: Returns tracked current step position.

### `void Stepper::moveSteps(int steps)`
- Purpose: Moves relative number of steps from current position.
- Behavior: Positive values move one direction, negative values move opposite direction.
