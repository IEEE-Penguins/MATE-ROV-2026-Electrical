#include <Arduino.h>
#include "thruster.h"

// =====================================================
// Serial
// =====================================================
static const unsigned long SERIAL_BAUD_RATE = 460800;
static const size_t RX_BUFFER_SIZE = 160;

// =====================================================
// Timing
// =====================================================
static const unsigned long COMMAND_TIMEOUT_MS = 1000;

// =====================================================
// Thruster objects
// =====================================================
Thruster gThruster1{ThrusterConfig(THRUSTER_1_PIN)};
Thruster gThruster2{ThrusterConfig(THRUSTER_2_PIN)};
Thruster gThruster3{ThrusterConfig(THRUSTER_3_PIN)};
Thruster gThruster4{ThrusterConfig(THRUSTER_4_PIN)};
Thruster gThruster5{ThrusterConfig(THRUSTER_5_PIN)};
Thruster gThruster6{ThrusterConfig(THRUSTER_6_PIN)};

Thruster* thrusterArray[THRUSTER_COUNT] = {
  &gThruster1,
  &gThruster2,
  &gThruster3,
  &gThruster4,
  &gThruster5,
  &gThruster6
};

Thrusters thrusters(thrusterArray, THRUSTER_COUNT);
bool thrustersReady = false;

// =====================================================
// Command state
// =====================================================
struct CommandState {
  float esc[THRUSTER_COUNT];
  bool valid;
};

CommandState commandState;

// =====================================================
// Runtime state
// =====================================================
char rxBuffer[RX_BUFFER_SIZE];
size_t rxIndex = 0;
unsigned long lastCommandMs = 0;

// =====================================================
// Helpers
// =====================================================
static float clampNormalized(float value) {
  if (value < -1.0f) return -1.0f;
  if (value >  1.0f) return  1.0f;
  return value;
}

static void resetCommandState(CommandState& state) {
  for (uint8_t i = 0; i < THRUSTER_COUNT; ++i) {
    state.esc[i] = 0.0f;
  }
  state.valid = false;
}

static void stopThrusters() {
  if (thrustersReady) {
    thrusters.stopAll();
  }
}

static void applyThrusters(const CommandState& state) {
  if (!thrustersReady || !state.valid) {
    return;
  }

  float commands[THRUSTER_COUNT];
  for (uint8_t i = 0; i < THRUSTER_COUNT; ++i) {
    commands[i] = clampNormalized(state.esc[i]);
  }

  thrusters.setAll(commands, THRUSTER_COUNT);
}

static void applyFailsafe() {
  resetCommandState(commandState);
  commandState.valid = true;
  stopThrusters();
}

// Parse only the E section:
// E,e1,e2,e3,e4,e5,e6
// Anything after that is ignored.
static bool parseCommandFrame(char* line, CommandState& outState) {
  char* token = strtok(line, ",");
  if (token == nullptr || strcmp(token, "E") != 0) {
    return false;
  }

  CommandState parsed;
  resetCommandState(parsed);

  for (uint8_t i = 0; i < THRUSTER_COUNT; ++i) {
    token = strtok(nullptr, ",");
    if (token == nullptr) {
      return false;
    }

    parsed.esc[i] = clampNormalized(static_cast<float>(atof(token)));
  }

  parsed.valid = true;
  outState = parsed;
  return true;
}

static void processIncomingSerial() {
  while (Serial.available() > 0) {
    const char c = static_cast<char>(Serial.read());

    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      rxBuffer[rxIndex] = '\0';

      if (rxIndex > 0) {
        CommandState parsed;
        if (parseCommandFrame(rxBuffer, parsed)) {
          commandState = parsed;
          lastCommandMs = millis();
        }
      }

      rxIndex = 0;
      rxBuffer[0] = '\0';
      continue;
    }

    if (rxIndex < (RX_BUFFER_SIZE - 1)) {
      rxBuffer[rxIndex++] = c;
    } else {
      rxIndex = 0;
      rxBuffer[0] = '\0';
    }
  }
}

static void beginThrusters() {
  thrusters.beginAll();
  thrusters.stopAll();
  thrustersReady = true;
}

// =====================================================
// Arduino setup / loop
// =====================================================
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  resetCommandState(commandState);
  rxIndex = 0;
  rxBuffer[0] = '\0';

  beginThrusters();
  applyFailsafe();

  lastCommandMs = millis();
}

void loop() {
  processIncomingSerial();

  const unsigned long now = millis();

  if ((now - lastCommandMs) > COMMAND_TIMEOUT_MS) {
    applyFailsafe();
    lastCommandMs = now;
  } else {
    applyThrusters(commandState);
  }
}