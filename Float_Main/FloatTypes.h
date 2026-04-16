#ifndef FLOAT_TYPES_H
#define FLOAT_TYPES_H

#include <Arduino.h>

struct MissionPacketContext {
  uint8_t profileIndex = 1;
  const char* phaseName = "AwaitFirstTransmit";
  bool inDeepRange = false;
  bool inShallowRange = false;
  bool profilePenalty = false;
  bool recoveryReady = false;
  uint8_t successfulProfiles = 0;
  uint8_t completedProfiles = 0;
  uint8_t holdSampleCount = 0;
};

#endif
