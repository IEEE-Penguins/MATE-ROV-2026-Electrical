#include "MissionController.h"

namespace {
const char* phaseToName(MissionPhase phase) {
  switch (phase) {
    case MissionPhase::kAwaitFirstTransmit:
      return "AwaitFirstTransmit";
    case MissionPhase::kDescendToDeep:
      return "DescendToDeep";
    case MissionPhase::kHoldDeep:
      return "HoldDeep";
    case MissionPhase::kAscendToShallow:
      return "AscendToShallow";
    case MissionPhase::kHoldShallow:
      return "HoldShallow";
    case MissionPhase::kProfileComplete:
      return "ProfileComplete";
    case MissionPhase::kRecoveryReady:
      return "RecoveryReady";
    default:
      return "Unknown";
  }
}
}  // namespace

MissionController::MissionController(Stepper& stepper)
    : stepper_(stepper),
      phase_(MissionPhase::kAwaitFirstTransmit),
      firstTransmitSuccess_(false),
      profileIndex_(1),
      successfulProfiles_(0),
      completedProfiles_(0),
      activeProfilePenalty_(false),
      missionComplete_(false),
      missionStartMs_(0),
      lastControlMs_(0),
      lastStepperStepMs_(0),
      holdStartMs_(0),
      lastHoldSampleMs_(0),
      holdSampleCount_(0),
      lastDepthMeters_(0.0f) {}

void MissionController::begin(uint32_t nowMs) {
  missionStartMs_ = nowMs;
  lastControlMs_ = nowMs;
  lastStepperStepMs_ = nowMs;

  phase_ = MissionPhase::kAwaitFirstTransmit;
  firstTransmitSuccess_ = false;
  profileIndex_ = 1;
  successfulProfiles_ = 0;
  completedProfiles_ = 0;
  activeProfilePenalty_ = false;
  missionComplete_ = false;

  resetHoldTracking();
}

void MissionController::markFirstTransmitSuccess() {
  if (firstTransmitSuccess_) {
    return;
  }

  firstTransmitSuccess_ = true;
  if (phase_ == MissionPhase::kAwaitFirstTransmit) {
    transitionTo(MissionPhase::kDescendToDeep, millis());
  }
}

bool MissionController::hasFirstTransmit() const {
  return firstTransmitSuccess_;
}

void MissionController::update(uint32_t nowMs, float depthMeters) {
  lastDepthMeters_ = depthMeters;

  if (missionComplete_) {
    return;
  }

  if ((uint32_t)(nowMs - missionStartMs_) >= FloatConfig::MISSION_DURATION_LIMIT_MS) {
    missionComplete_ = true;
    transitionTo(MissionPhase::kRecoveryReady, nowMs);
    return;
  }

  if ((uint32_t)(nowMs - lastControlMs_) < FloatConfig::CONTROL_INTERVAL_MS) {
    return;
  }
  lastControlMs_ = nowMs;

  if (phase_ != MissionPhase::kAwaitFirstTransmit && phase_ != MissionPhase::kRecoveryReady) {
    updatePenalty(depthMeters);
  }

  switch (phase_) {
    case MissionPhase::kAwaitFirstTransmit:
      controlTowardRange(nowMs, depthMeters, FloatConfig::SHALLOW_SENSOR_MIN_M, FloatConfig::SHALLOW_SENSOR_MAX_M);
      if (firstTransmitSuccess_) {
        transitionTo(MissionPhase::kDescendToDeep, nowMs);
      }
      break;

    case MissionPhase::kDescendToDeep:
      controlTowardRange(nowMs, depthMeters, FloatConfig::DEEP_MIN_M, FloatConfig::DEEP_MAX_M);
      if (inDeepRange(depthMeters)) {
        transitionTo(MissionPhase::kHoldDeep, nowMs);
      }
      break;

    case MissionPhase::kHoldDeep:
      processHoldState(nowMs, depthMeters, true);
      break;

    case MissionPhase::kAscendToShallow:
      controlTowardRange(nowMs, depthMeters, FloatConfig::SHALLOW_SENSOR_MIN_M, FloatConfig::SHALLOW_SENSOR_MAX_M);
      if (inShallowRange(depthMeters)) {
        transitionTo(MissionPhase::kHoldShallow, nowMs);
      }
      break;

    case MissionPhase::kHoldShallow:
      processHoldState(nowMs, depthMeters, false);
      break;

    case MissionPhase::kProfileComplete:
      finalizeProfile(nowMs);
      break;

    case MissionPhase::kRecoveryReady:
      controlTowardRange(nowMs, depthMeters, FloatConfig::SHALLOW_SENSOR_MIN_M, FloatConfig::SHALLOW_SENSOR_MAX_M);
      break;
  }
}

void MissionController::onSampleTick(uint32_t nowMs, float depthMeters) {
  bool holdDeep = (phase_ == MissionPhase::kHoldDeep);
  bool holdShallow = (phase_ == MissionPhase::kHoldShallow);

  if (!holdDeep && !holdShallow) {
    return;
  }

  bool inRange = holdDeep ? inDeepRange(depthMeters) : inShallowRange(depthMeters);
  if (!inRange) {
    resetHoldTracking();
    return;
  }

  if (holdStartMs_ == 0) {
    holdStartMs_ = nowMs;
  }

  if (holdSampleCount_ == 0) {
    holdSampleCount_ = 1;
    lastHoldSampleMs_ = nowMs;
    return;
  }

  uint32_t elapsedSinceLastSample = (uint32_t)(nowMs - lastHoldSampleMs_);
  if (elapsedSinceLastSample + 500 >= FloatConfig::SAMPLE_INTERVAL_MS) {
    holdSampleCount_++;
    lastHoldSampleMs_ = nowMs;
  }
}

MissionPacketContext MissionController::packetContext() const {
  MissionPacketContext context;
  context.profileIndex = profileIndex_;
  context.phaseName = phaseName();
  context.inDeepRange = inDeepRange(lastDepthMeters_);
  context.inShallowRange = inShallowRange(lastDepthMeters_);
  context.profilePenalty = activeProfilePenalty_;
  context.recoveryReady = (phase_ == MissionPhase::kRecoveryReady);
  context.successfulProfiles = successfulProfiles_;
  context.completedProfiles = completedProfiles_;
  context.holdSampleCount = holdSampleCount_;
  return context;
}

MissionPhase MissionController::phase() const {
  return phase_;
}

const char* MissionController::phaseName() const {
  return phaseToName(phase_);
}

uint8_t MissionController::currentProfile() const {
  return profileIndex_;
}

uint8_t MissionController::successfulProfiles() const {
  return successfulProfiles_;
}

uint8_t MissionController::completedProfiles() const {
  return completedProfiles_;
}

bool MissionController::profilePenaltyActive() const {
  return activeProfilePenalty_;
}

bool MissionController::isRecoveryReady() const {
  return phase_ == MissionPhase::kRecoveryReady;
}

uint8_t MissionController::currentHoldSampleCount() const {
  return holdSampleCount_;
}

uint32_t MissionController::currentHoldElapsedMs(uint32_t nowMs) const {
  if (holdStartMs_ == 0) {
    return 0;
  }
  return (uint32_t)(nowMs - holdStartMs_);
}

bool MissionController::inDeepRange(float depthMeters) const {
  return depthMeters >= FloatConfig::DEEP_MIN_M && depthMeters <= FloatConfig::DEEP_MAX_M;
}

bool MissionController::inShallowRange(float depthMeters) const {
  return depthMeters >= FloatConfig::SHALLOW_SENSOR_MIN_M && depthMeters <= FloatConfig::SHALLOW_SENSOR_MAX_M;
}

void MissionController::transitionTo(MissionPhase nextPhase, uint32_t nowMs) {
  (void)nowMs;
  phase_ = nextPhase;

  if (phase_ == MissionPhase::kHoldDeep || phase_ == MissionPhase::kHoldShallow) {
    resetHoldTracking();
    return;
  }

  if (phase_ == MissionPhase::kRecoveryReady) {
    resetHoldTracking();
    return;
  }

  if (phase_ == MissionPhase::kDescendToDeep || phase_ == MissionPhase::kAscendToShallow ||
      phase_ == MissionPhase::kProfileComplete) {
    resetHoldTracking();
  }
}

void MissionController::resetHoldTracking() {
  holdStartMs_ = 0;
  lastHoldSampleMs_ = 0;
  holdSampleCount_ = 0;
}

void MissionController::updatePenalty(float depthMeters) {
  if (depthMeters <= FloatConfig::SURFACE_CONTACT_SENSOR_DEPTH_M) {
    activeProfilePenalty_ = true;
  }
}

void MissionController::processHoldState(uint32_t nowMs, float depthMeters, bool deepHold) {
  float minDepth = deepHold ? FloatConfig::DEEP_MIN_M : FloatConfig::SHALLOW_SENSOR_MIN_M;
  float maxDepth = deepHold ? FloatConfig::DEEP_MAX_M : FloatConfig::SHALLOW_SENSOR_MAX_M;

  controlTowardRange(nowMs, depthMeters, minDepth, maxDepth);

  bool inRange = deepHold ? inDeepRange(depthMeters) : inShallowRange(depthMeters);
  if (!inRange) {
    resetHoldTracking();
    return;
  }

  if (holdStartMs_ == 0) {
    return;
  }

  uint32_t holdElapsed = (uint32_t)(nowMs - holdStartMs_);
  if (holdElapsed < FloatConfig::HOLD_DURATION_MS) {
    return;
  }

  if (holdSampleCount_ < FloatConfig::REQUIRED_HOLD_SAMPLES) {
    return;
  }

  if (deepHold) {
    transitionTo(MissionPhase::kAscendToShallow, nowMs);
  } else {
    transitionTo(MissionPhase::kProfileComplete, nowMs);
  }
}

void MissionController::finalizeProfile(uint32_t nowMs) {
  completedProfiles_++;

  if (!activeProfilePenalty_) {
    successfulProfiles_++;
  }

  if (successfulProfiles_ >= FloatConfig::TARGET_SUCCESSFUL_PROFILES) {
    missionComplete_ = true;
    transitionTo(MissionPhase::kRecoveryReady, nowMs);
    return;
  }

  if (completedProfiles_ >= FloatConfig::MAX_PROFILE_ATTEMPTS) {
    missionComplete_ = true;
    transitionTo(MissionPhase::kRecoveryReady, nowMs);
    return;
  }

  profileIndex_++;
  activeProfilePenalty_ = false;
  transitionTo(MissionPhase::kDescendToDeep, nowMs);
}

void MissionController::controlTowardRange(uint32_t nowMs, float depthMeters, float minDepth, float maxDepth) {
  if ((uint32_t)(nowMs - lastStepperStepMs_) < FloatConfig::STEPPER_STEP_INTERVAL_MS) {
    return;
  }

  if (depthMeters > (maxDepth + FloatConfig::RANGE_MARGIN_M)) {
    stepTowardShallower(nowMs);
    return;
  }

  if (depthMeters < (minDepth - FloatConfig::RANGE_MARGIN_M)) {
    stepTowardDeeper(nowMs);
    return;
  }

  float center = (minDepth + maxDepth) * 0.5f;
  if (depthMeters > (center + FloatConfig::CENTER_TRIM_MARGIN_M)) {
    stepTowardShallower(nowMs);
  } else if (depthMeters < (center - FloatConfig::CENTER_TRIM_MARGIN_M)) {
    stepTowardDeeper(nowMs);
  }
}

void MissionController::stepTowardDeeper(uint32_t nowMs) {
  stepper_.moveSingleDirection(FloatConfig::STEPPER_DIR_FOR_DESCEND);
  lastStepperStepMs_ = nowMs;
}

void MissionController::stepTowardShallower(uint32_t nowMs) {
  stepper_.moveSingleDirection(!FloatConfig::STEPPER_DIR_FOR_DESCEND);
  lastStepperStepMs_ = nowMs;
}
