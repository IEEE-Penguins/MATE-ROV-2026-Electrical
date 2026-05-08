#ifndef MISSION_CONTROLLER_H
#define MISSION_CONTROLLER_H

#include <Arduino.h>

#include "FloatConfig.h"
#include "FloatTypes.h"
#include "Stepper.h"

enum class MissionPhase {
  kAwaitFirstTransmit,
  kDescendToDeep,
  kHoldDeep,
  kAscendToShallow,
  kHoldShallow,
  kProfileComplete,
  kRecoveryReady,
};

class MissionController {
  public:
    explicit MissionController(Stepper& stepper);

    void begin(uint32_t nowMs);
    void update(uint32_t nowMs, float depthMeters);
    void onSampleTick(uint32_t nowMs, float depthMeters);

    void markFirstTransmitSuccess();
    bool hasFirstTransmit() const;

    MissionPacketContext packetContext() const;

    MissionPhase phase() const;
    const char* phaseName() const;

    uint8_t currentProfile() const;
    uint8_t successfulProfiles() const;
    uint8_t completedProfiles() const;
    bool profilePenaltyActive() const;
    bool isRecoveryReady() const;

    uint8_t currentHoldSampleCount() const;
    uint32_t currentHoldElapsedMs(uint32_t nowMs) const;

    bool inDeepRange(float depthMeters) const;
    bool inShallowRange(float depthMeters) const;

  private:
    Stepper& stepper_;

    MissionPhase phase_;
    bool firstTransmitSuccess_;

    uint8_t profileIndex_;
    uint8_t successfulProfiles_;
    uint8_t completedProfiles_;

    bool activeProfilePenalty_;
    bool missionComplete_;

    uint32_t missionStartMs_;
    uint32_t lastControlMs_;
    uint32_t lastStepperStepMs_;

    uint32_t holdStartMs_;
    uint32_t lastHoldSampleMs_;
    uint8_t holdSampleCount_;
    float lastDepthMeters_;

    void transitionTo(MissionPhase nextPhase, uint32_t nowMs);
    void resetHoldTracking();

    void updatePenalty(float depthMeters);
    void processHoldState(uint32_t nowMs, float depthMeters, bool deepHold);
    void finalizeProfile(uint32_t nowMs);

    void controlTowardRange(uint32_t nowMs, float depthMeters, float minDepth, float maxDepth);
    void stepTowardDeeper(uint32_t nowMs);
    void stepTowardShallower(uint32_t nowMs);
};

#endif
