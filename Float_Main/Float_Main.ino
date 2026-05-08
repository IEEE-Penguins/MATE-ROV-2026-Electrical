#include "CommsLink.h"
#include "Depth.h"
#include "FloatConfig.h"
#include "MissionController.h"
#include "Stepper.h"

Depth depthSensor;
Stepper ballastStepper;
MissionController missionController(ballastStepper);
CommsLink commsLink;

uint32_t lastSampleAt = 0;
uint32_t lastStatusPrintAt = 0;

float readDepthMeters() {
  float depthMeters = depthSensor.getDepthMeters();

  if (depthMeters < 0.0f) {
    depthMeters = 0.0f;
  }

  if (depthMeters > FloatConfig::MAX_VALID_SENSOR_DEPTH_M) {
    depthMeters = FloatConfig::MAX_VALID_SENSOR_DEPTH_M;
  }

  return depthMeters;
}

void printRuntimeStatus(uint32_t nowMs, float depthMeters) {
  MissionPacketContext context = missionController.packetContext();

  Serial.printf(
      "[STATE] phase=%s profile=%u completed=%u success=%u penalty=%d depth=%.2f inDeep=%d "
      "inShallow=%d holdSamples=%u holdMs=%lu queue=%u tx=%d\n",
      missionController.phaseName(), context.profileIndex, context.completedProfiles,
      context.successfulProfiles, context.profilePenalty ? 1 : 0, depthMeters,
      context.inDeepRange ? 1 : 0, context.inShallowRange ? 1 : 0, context.holdSampleCount,
      (unsigned long)missionController.currentHoldElapsedMs(nowMs), (unsigned int)commsLink.queuedCount(),
      commsLink.hasSuccessfulTransmit() ? 1 : 0);
}

void setup() {
  Serial.begin(115200);
  delay(400);

  Serial.println();
  Serial.println("=== Float Main Mission Controller ===");

  depthSensor.begin(FloatConfig::PIN_DEPTH_DOUT, FloatConfig::PIN_DEPTH_SCK);
  depthSensor.setCalibrationFactor(FloatConfig::DEPTH_CALIBRATION_FACTOR);
  depthSensor.setSamples(FloatConfig::DEPTH_READ_SAMPLES);

  Serial.println("[DEPTH] Calibrating depth sensor at startup...");
  depthSensor.calibrate(FloatConfig::DEPTH_CALIBRATE_SAMPLES);
  Serial.println("[DEPTH] Calibration complete.");

  ballastStepper.begin((short)FloatConfig::PIN_STEPPER_DIR, (short)FloatConfig::PIN_STEPPER_STEP);
  ballastStepper.setSpeed(FloatConfig::STEPPER_STEPS_PER_SECOND);

  missionController.begin(millis());
  commsLink.begin();

  uint32_t nowMs = millis();
  lastSampleAt = nowMs - FloatConfig::SAMPLE_INTERVAL_MS;
  lastStatusPrintAt = nowMs;
}

void loop() {
  uint32_t nowMs = millis();
  float depthMeters = readDepthMeters();

  commsLink.update(nowMs);

  if (commsLink.hasSuccessfulTransmit() && !missionController.hasFirstTransmit()) {
    missionController.markFirstTransmitSuccess();
  }

  missionController.update(nowMs, depthMeters);

  if ((uint32_t)(nowMs - lastSampleAt) >= FloatConfig::SAMPLE_INTERVAL_MS) {
    lastSampleAt = nowMs;

    missionController.onSampleTick(nowMs, depthMeters);

    bool sentNow = commsLink.sendSample(depthMeters, missionController.packetContext(), nowMs);
    if (sentNow && !missionController.hasFirstTransmit()) {
      missionController.markFirstTransmitSuccess();
    }
  }

  if ((uint32_t)(nowMs - lastStatusPrintAt) >= FloatConfig::STATUS_PRINT_INTERVAL_MS) {
    lastStatusPrintAt = nowMs;
    printRuntimeStatus(nowMs, depthMeters);
  }

  delay(10);
}
