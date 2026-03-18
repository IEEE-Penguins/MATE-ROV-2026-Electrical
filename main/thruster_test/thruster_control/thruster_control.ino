#include <Arduino.h>
#include <ArduinoJson.h>
#include "thruster.h"

// =====================================================
// Serial
// =====================================================
static const unsigned long SERIAL_BAUD_RATE = 115200;
static const size_t RX_BUFFER_SIZE = 256;

// =====================================================
// Timing
// =====================================================
static const unsigned long TELEMETRY_INTERVAL_MS = 100;
static const unsigned long COMMAND_TIMEOUT_MS    = 1000;
static const unsigned long STARTUP_QUIET_MS      = 1500;

// =====================================================
// Thruster shared-signal rule
// =====================================================
// 6 physical thrusters, but one pair shares the same signal.
// ESC[5] mirrors ESC[4].
static const uint8_t THRUSTER_SHARED_SOURCE_INDEX = 4;
static const uint8_t THRUSTER_SHARED_MIRROR_INDEX = 5;

// =====================================================
// Protocol sizes
// =====================================================
static const uint8_t COMMAND_ESC_COUNT    = 6;
static const uint8_t COMMAND_SERVO_COUNT  = 4;
static const uint8_t COMMAND_LIGHTS_COUNT = 2;

// =====================================================
// JSON sizes
// =====================================================
static const size_t COMMAND_JSON_CAPACITY   = 384;
static const size_t TELEMETRY_JSON_CAPACITY = 192;

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
  int8_t servo[COMMAND_SERVO_COUNT];
  uint8_t lights[COMMAND_LIGHTS_COUNT];
  bool valid;
};

CommandState commandState;

// =====================================================
// Runtime state
// =====================================================
char rxBuffer[RX_BUFFER_SIZE];
size_t rxIndex = 0;

unsigned long bootMs          = 0;
unsigned long lastTelemetryMs = 0;
unsigned long lastCommandMs   = 0;

// =====================================================
// Helpers
// =====================================================
static float clampNormalized(float value) {
  if (value < -1.0f) return -1.0f;
  if (value >  1.0f) return  1.0f;
  return value;
}

static int8_t clampServoCommand(int value) {
  if (value < -1) return -1;
  if (value >  1) return  1;
  return static_cast<int8_t>(value);
}

static uint8_t clampLightCommand(int value) {
  return (value != 0) ? 1 : 0;
}

static void resetCommandState(CommandState& state) {
  for (uint8_t i = 0; i < THRUSTER_COUNT; ++i) {
    state.esc[i] = 0.0f;
  }

  for (uint8_t i = 0; i < COMMAND_SERVO_COUNT; ++i) {
    state.servo[i] = 0;
  }

  for (uint8_t i = 0; i < COMMAND_LIGHTS_COUNT; ++i) {
    state.lights[i] = 0;
  }

  state.valid = false;
}

static void applySharedThrusterRule(float esc[THRUSTER_COUNT]) {
  if (THRUSTER_SHARED_SOURCE_INDEX < THRUSTER_COUNT &&
      THRUSTER_SHARED_MIRROR_INDEX < THRUSTER_COUNT) {
    esc[THRUSTER_SHARED_MIRROR_INDEX] = esc[THRUSTER_SHARED_SOURCE_INDEX];
  }
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

  applySharedThrusterRule(commands);
  thrusters.setAll(commands, THRUSTER_COUNT);
}

static void applyFailsafe() {
  resetCommandState(commandState);
  commandState.valid = true;
  stopThrusters();
}

static bool isNumericJson(JsonVariantConst v) {
  return v.is<int>() || v.is<long>() || v.is<float>() || v.is<double>();
}

static bool parseCommandFrame(const char* line, CommandState& outState) {
  StaticJsonDocument<COMMAND_JSON_CAPACITY> doc;
  DeserializationError error = deserializeJson(doc, line);
  if (error) {
    return false;
  }

  const char* type = doc["type"];
  if (type == nullptr || strcmp(type, "command") != 0) {
    return false;
  }

  JsonObjectConst data = doc["data"].as<JsonObjectConst>();
  if (data.isNull()) {
    return false;
  }

  JsonArrayConst escArray = data["esc"].as<JsonArrayConst>();
  JsonArrayConst servoArray = data["servo"].as<JsonArrayConst>();
  JsonArrayConst lightsArray = data["lights"].as<JsonArrayConst>();

  if (escArray.isNull() || escArray.size() != COMMAND_ESC_COUNT) {
    return false;
  }

  if (servoArray.isNull() || servoArray.size() != COMMAND_SERVO_COUNT) {
    return false;
  }

  if (lightsArray.isNull() || lightsArray.size() != COMMAND_LIGHTS_COUNT) {
    return false;
  }

  CommandState parsed;
  resetCommandState(parsed);

  for (uint8_t i = 0; i < COMMAND_ESC_COUNT; ++i) {
    if (!isNumericJson(escArray[i])) {
      return false;
    }
    parsed.esc[i] = clampNormalized(escArray[i].as<float>());
  }

  for (uint8_t i = 0; i < COMMAND_SERVO_COUNT; ++i) {
    if (!servoArray[i].is<int>() && !servoArray[i].is<long>()) {
      return false;
    }
    parsed.servo[i] = clampServoCommand(servoArray[i].as<int>());
  }

  for (uint8_t i = 0; i < COMMAND_LIGHTS_COUNT; ++i) {
    if (!lightsArray[i].is<int>() && !lightsArray[i].is<long>()) {
      return false;
    }
    parsed.lights[i] = clampLightCommand(lightsArray[i].as<int>());
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

static void publishTelemetry() {
  StaticJsonDocument<TELEMETRY_JSON_CAPACITY> out;

  out["type"] = "sensors";
  JsonObject data = out["data"].to<JsonObject>();

  data["depth"] = 0.0f;

  JsonObject mpu = data["mpu"].to<JsonObject>();

  JsonArray acc = mpu["acc"].to<JsonArray>();
  acc.add(0.0f);
  acc.add(0.0f);
  acc.add(0.0f);

  JsonArray gyro = mpu["gyro"].to<JsonArray>();
  gyro.add(0.0f);
  gyro.add(0.0f);
  gyro.add(0.0f);

  JsonArray angle = mpu["angle"].to<JsonArray>();
  angle.add(0.0f);
  angle.add(0.0f);
  angle.add(0.0f);

  mpu["temp_in"] = 0.0f;

  serializeJson(out, Serial);
  Serial.write('\n');
}

static void beginThrusters() {
  thrusters.beginAll();
  thrusters.stopAll();
  thrustersReady = true;
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  resetCommandState(commandState);

  rxIndex = 0;
  rxBuffer[0] = '\0';

  beginThrusters();
  applyFailsafe();

  bootMs = millis();
  lastCommandMs = bootMs;
  lastTelemetryMs = bootMs;
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

  if ((now - bootMs) >= STARTUP_QUIET_MS &&
      (now - lastTelemetryMs) >= TELEMETRY_INTERVAL_MS) {
    lastTelemetryMs = now;
    publishTelemetry();
  }
}