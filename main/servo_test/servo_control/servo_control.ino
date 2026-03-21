#include <Arduino.h>
#include "servo.h"

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
// Servo command mapping
// =====================================================
// servo[0] -> positional servo 1 (pin 32) : angle in degrees, constrained to 116..180
// servo[1] -> positional servo 2 (pin 33) : angle in degrees, constrained to 0..180
// servo[2] -> continuous servo 1 (pin 25) : speed in range -1.0..1.0
// servo[3] -> continuous servo 2 (pin 26) : speed in range -1.0..1.0

static const float SERVO1_MIN_DEG  = 116.0f;  // from your hardware note
static const float SERVO1_MAX_DEG  = 180.0f;
static const float SERVO1_HOME_DEG = 116.0f;

static const float SERVO2_MIN_DEG  = 0.0f;
static const float SERVO2_MAX_DEG  = 180.0f;
static const float SERVO2_HOME_DEG = 90.0f;

// =====================================================
// Servo objects
// =====================================================
PositionalServo servo1(
    ServoConfig::POSITIONAL_SERVO_1_PIN,
    SERVO1_MIN_DEG,
    SERVO1_MAX_DEG,
    SERVO1_HOME_DEG,
    ServoConfig::DEFAULT_MIN_PULSE_US,
    ServoConfig::DEFAULT_MAX_PULSE_US
);

PositionalServo servo2(
    ServoConfig::POSITIONAL_SERVO_2_PIN,
    SERVO2_MIN_DEG,
    SERVO2_MAX_DEG,
    SERVO2_HOME_DEG,
    ServoConfig::DEFAULT_MIN_PULSE_US,
    ServoConfig::DEFAULT_MAX_PULSE_US
);

ContinuousServo servo3(
    ServoConfig::CONTINUOUS_SERVO_1_PIN,
    ServoConfig::CONTINUOUS_MIN_PULSE_US,
    ServoConfig::CONTINUOUS_NEUTRAL_PULSE_US,
    ServoConfig::CONTINUOUS_MAX_PULSE_US
);

ContinuousServo servo4(
    ServoConfig::CONTINUOUS_SERVO_2_PIN,
    ServoConfig::CONTINUOUS_MIN_PULSE_US,
    ServoConfig::CONTINUOUS_NEUTRAL_PULSE_US,
    ServoConfig::CONTINUOUS_MAX_PULSE_US
);

bool servosReady = false;

// =====================================================
// Command state
// =====================================================
struct ServoCommandState {
  float values[4];
  bool valid;
};

ServoCommandState commandState;

// =====================================================
// Runtime state
// =====================================================
char rxBuffer[RX_BUFFER_SIZE];
size_t rxIndex = 0;
unsigned long lastCommandMs = 0;

// =====================================================
// Helpers
// =====================================================
static float clampFloat(float value, float minValue, float maxValue) {
  if (value < minValue) return minValue;
  if (value > maxValue) return maxValue;
  return value;
}

static void resetCommandState(ServoCommandState& state) {
  for (uint8_t i = 0; i < 4; ++i) {
    state.values[i] = 0.0f;
  }
  state.valid = false;
}

static void moveToSafeState() {
  if (!servosReady) {
    return;
  }

  servo1.moveHome();
  servo2.moveHome();
  servo3.stop();
  servo4.stop();
}

static void applyServoCommands(const ServoCommandState& state) {
  if (!servosReady || !state.valid) {
    return;
  }

  const float s1Angle = clampFloat(state.values[0], SERVO1_MIN_DEG, SERVO1_MAX_DEG);
  const float s2Angle = clampFloat(state.values[1], SERVO2_MIN_DEG, SERVO2_MAX_DEG);
  const float s3Speed = clampFloat(state.values[2], -1.0f, 1.0f);
  const float s4Speed = clampFloat(state.values[3], -1.0f, 1.0f);

  servo1.setAngle(s1Angle);
  servo2.setAngle(s2Angle);
  servo3.setSpeed(s3Speed);
  servo4.setSpeed(s4Speed);
}

static void applyFailsafe() {
  resetCommandState(commandState);
  commandState.valid = true;
  moveToSafeState();
}

// Parse only the S section from:
// E,e1,e2,e3,e4,e5,e6,S,s1,s2,s3,s4,L,l1,l2
static bool parseServoFrame(char* line, ServoCommandState& outState) {
  char* token = strtok(line, ",");
  if (token == nullptr || strcmp(token, "E") != 0) {
    return false;
  }

  // Skip 6 ESC fields
  for (uint8_t i = 0; i < 6; ++i) {
    token = strtok(nullptr, ",");
    if (token == nullptr) {
      return false;
    }
  }

  token = strtok(nullptr, ",");
  if (token == nullptr || strcmp(token, "S") != 0) {
    return false;
  }

  ServoCommandState parsed;
  resetCommandState(parsed);

  for (uint8_t i = 0; i < 4; ++i) {
    token = strtok(nullptr, ",");
    if (token == nullptr) {
      return false;
    }

    parsed.values[i] = static_cast<float>(atof(token));
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
        ServoCommandState parsed;
        if (parseServoFrame(rxBuffer, parsed)) {
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

static void beginServos() {
  servo1.begin();
  servo2.begin();
  servo3.begin();
  servo4.begin();
  servosReady = true;
  moveToSafeState();
}

// =====================================================
// Arduino setup / loop
// =====================================================
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  resetCommandState(commandState);
  rxIndex = 0;
  rxBuffer[0] = '\0';

  beginServos();
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
    applyServoCommands(commandState);
  }
}