#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>

#include "mpu.h"
#include "depth.h"
#include "thruster.h"
#include "servo.h"

// =====================================================
// Serial protocol
// =====================================================
static const unsigned long SERIAL_BAUD_RATE = 115200;
static const size_t RX_BUFFER_SIZE = 384;

// =====================================================
// Timing
// =====================================================
static const unsigned long TELEMETRY_INTERVAL_MS   = 100;
static const unsigned long COMMAND_TIMEOUT_MS      = 1000;
static const unsigned long SERVO_STEP_INTERVAL_MS  = 20;

// =====================================================
// Lights
// =====================================================
static const uint8_t LIGHT_PIN = 14;

// =====================================================
// Thruster shared-signal assumption
// =====================================================
// There are 6 physical thrusters. Two share the same final signal.
// Current assumption: ESC[5] mirrors ESC[4].
static const uint8_t THRUSTER_SHARED_SOURCE_INDEX = 4;
static const uint8_t THRUSTER_SHARED_MIRROR_INDEX = 5;

// =====================================================
// Servo behavior
// =====================================================
static const float SERVO_COMMAND_NEGATIVE = -1.0f;
static const float SERVO_COMMAND_POSITIVE =  1.0f;

static const float POSITIONAL_SERVO_1_MIN_DEG  = 116.0f;
static const float POSITIONAL_SERVO_1_MAX_DEG  = 180.0f;
static const float POSITIONAL_SERVO_1_HOME_DEG = 148.0f;

static const float POSITIONAL_SERVO_2_MIN_DEG  = ServoConfig::POSITIONAL_MIN_ANGLE_DEG;
static const float POSITIONAL_SERVO_2_MAX_DEG  = ServoConfig::POSITIONAL_MAX_ANGLE_DEG;
static const float POSITIONAL_SERVO_2_HOME_DEG = ServoConfig::POSITIONAL_HOME_ANGLE_DEG;

static const float POSITIONAL_SERVO_STEP_DEG = 2.0f;
static const float CONTINUOUS_SERVO_SPEED    = 1.0f;

// =====================================================
// JSON document sizes
// =====================================================
static const size_t COMMAND_JSON_CAPACITY   = 384;
static const size_t TELEMETRY_JSON_CAPACITY = 512;

// =====================================================
// Sensor modules
// =====================================================
MPU6050 mpu(Wire);
DepthSensor depthSensor(Wire);

bool mpuReady   = false;
bool depthReady = false;

// =====================================================
// Actuator modules
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

PositionalServo positionalServo1(
  ServoConfig::POSITIONAL_SERVO_1_PIN,
  POSITIONAL_SERVO_1_MIN_DEG,
  POSITIONAL_SERVO_1_MAX_DEG,
  POSITIONAL_SERVO_1_HOME_DEG
);

PositionalServo positionalServo2(
  ServoConfig::POSITIONAL_SERVO_2_PIN,
  POSITIONAL_SERVO_2_MIN_DEG,
  POSITIONAL_SERVO_2_MAX_DEG,
  POSITIONAL_SERVO_2_HOME_DEG
);

ContinuousServo continuousServo1(ServoConfig::CONTINUOUS_SERVO_1_PIN);
ContinuousServo continuousServo2(ServoConfig::CONTINUOUS_SERVO_2_PIN);

bool thrustersReady = false;
bool servosReady    = false;

// =====================================================
// Command state
// =====================================================
struct CommandState {
  float esc[THRUSTER_COUNT];
  float servo[ServoConfig::TOTAL_SERVO_COUNT];
  int lights[2];
  bool valid;
};

CommandState commandState;

// =====================================================
// Runtime state
// =====================================================
char rxBuffer[RX_BUFFER_SIZE];
size_t rxIndex = 0;

unsigned long lastTelemetryMs = 0;
unsigned long lastCommandMs   = 0;
unsigned long lastServoStepMs = 0;

// =====================================================
// Telemetry cache
// =====================================================
float telemetryDepthMeters = 0.0f;
float telemetryAcc[3]      = {0.0f, 0.0f, 0.0f};
float telemetryGyro[3]     = {0.0f, 0.0f, 0.0f};
float telemetryAngle[3]    = {0.0f, 0.0f, 0.0f};
float telemetryTempIn      = 0.0f;

// =====================================================
// Helpers
// =====================================================
static float degToRad(float deg) {
  return deg * PI / 180.0f;
}

static float gToMps2(float g) {
  return g * 9.80665f;
}

static float clampNormalized(float value) {
  if (value < -1.0f) return -1.0f;
  if (value >  1.0f) return  1.0f;
  return value;
}

static int clampBinaryLikeInt(JsonVariant value) {
  if (!value.is<int>() && !value.is<long>() && !value.is<float>() && !value.is<double>()) {
    return 0;
  }

  const float v = value.as<float>();
  if (v > 0.5f)  return 1;
  if (v < -0.5f) return -1;
  return 0;
}

static float clampBinaryLikeFloat(JsonVariant value) {
  if (!value.is<int>() && !value.is<long>() && !value.is<float>() && !value.is<double>()) {
    return 0.0f;
  }

  const float v = value.as<float>();
  if (v > 0.5f)  return 1.0f;
  if (v < -0.5f) return -1.0f;
  return 0.0f;
}

static void resetCommandState(CommandState &state) {
  for (uint8_t i = 0; i < THRUSTER_COUNT; ++i) {
    state.esc[i] = 0.0f;
  }

  for (uint8_t i = 0; i < ServoConfig::TOTAL_SERVO_COUNT; ++i) {
    state.servo[i] = 0.0f;
  }

  state.lights[0] = 0;
  state.lights[1] = 0;
  state.valid = false;
}

static void applySharedThrusterRule(float esc[THRUSTER_COUNT]) {
  if (THRUSTER_SHARED_SOURCE_INDEX < THRUSTER_COUNT &&
      THRUSTER_SHARED_MIRROR_INDEX < THRUSTER_COUNT) {
    esc[THRUSTER_SHARED_MIRROR_INDEX] = esc[THRUSTER_SHARED_SOURCE_INDEX];
  }
}

static void applyLights(const CommandState &state) {
  const bool lightOn = (state.lights[0] != 0);
  digitalWrite(LIGHT_PIN, lightOn ? HIGH : LOW);
}

static void applyThrusters(const CommandState &state) {
  if (!thrustersReady) {
    return;
  }

  float commands[THRUSTER_COUNT];

  for (uint8_t i = 0; i < THRUSTER_COUNT; ++i) {
    commands[i] = clampNormalized(state.esc[i]);
  }

  applySharedThrusterRule(commands);
  thrusters.setAll(commands, THRUSTER_COUNT);
}

static void applyContinuousServos(const CommandState &state) {
  if (!servosReady) {
    return;
  }

  continuousServo1.setSpeed(clampNormalized(state.servo[2]) * CONTINUOUS_SERVO_SPEED);
  continuousServo2.setSpeed(clampNormalized(state.servo[3]) * CONTINUOUS_SERVO_SPEED);
}

static void stepPositionalServo(PositionalServo &servo, float command) {
  if (command >= SERVO_COMMAND_POSITIVE) {
    servo.stepBy(POSITIONAL_SERVO_STEP_DEG);
  } else if (command <= SERVO_COMMAND_NEGATIVE) {
    servo.stepBy(-POSITIONAL_SERVO_STEP_DEG);
  }
}

static void updatePositionalServos(const CommandState &state) {
  if (!servosReady) {
    return;
  }

  const unsigned long now = millis();
  if ((now - lastServoStepMs) < SERVO_STEP_INTERVAL_MS) {
    return;
  }

  lastServoStepMs = now;

  stepPositionalServo(positionalServo1, state.servo[0]);
  stepPositionalServo(positionalServo2, state.servo[1]);
}

static void applyCommandState() {
  applyThrusters(commandState);
  applyContinuousServos(commandState);
  applyLights(commandState);
}

static void moveServosToSafeHome() {
  if (!servosReady) {
    return;
  }

  positionalServo1.moveHome();
  positionalServo2.moveHome();
  continuousServo1.stop();
  continuousServo2.stop();
}

static void applyFailsafe() {
  resetCommandState(commandState);
  commandState.valid = true;

  if (thrustersReady) {
    thrusters.stopAll();
  }

  if (servosReady) {
    moveServosToSafeHome();
  }

  digitalWrite(LIGHT_PIN, LOW);
}

static bool parseCommandFrame(const char *line, CommandState &outState) {
  StaticJsonDocument<COMMAND_JSON_CAPACITY> doc;
  DeserializationError error = deserializeJson(doc, line);
  if (error) {
    return false;
  }

  const char *type = doc["type"];
  if (type == nullptr || strcmp(type, "command") != 0) {
    return false;
  }

  JsonObject data = doc["data"].as<JsonObject>();
  if (data.isNull()) {
    return false;
  }

  JsonArray escArray    = data["esc"].as<JsonArray>();
  JsonArray servoArray  = data["servo"].as<JsonArray>();
  JsonArray lightsArray = data["lights"].as<JsonArray>();

  if (escArray.isNull() || servoArray.isNull() || lightsArray.isNull()) {
    return false;
  }

  if (escArray.size() != THRUSTER_COUNT ||
      servoArray.size() != ServoConfig::TOTAL_SERVO_COUNT ||
      lightsArray.size() != 2) {
    return false;
  }

  CommandState parsed;
  resetCommandState(parsed);

  for (uint8_t i = 0; i < THRUSTER_COUNT; ++i) {
    if (!escArray[i].is<int>() && !escArray[i].is<long>() &&
        !escArray[i].is<float>() && !escArray[i].is<double>()) {
      return false;
    }
    parsed.esc[i] = clampNormalized(escArray[i].as<float>());
  }

  for (uint8_t i = 0; i < ServoConfig::TOTAL_SERVO_COUNT; ++i) {
    parsed.servo[i] = clampBinaryLikeFloat(servoArray[i]);
  }

  parsed.lights[0] = clampBinaryLikeInt(lightsArray[0]);
  parsed.lights[1] = clampBinaryLikeInt(lightsArray[1]);
  parsed.valid = true;

  outState = parsed;
  return true;
}

static void updateTelemetryCache() {
  telemetryDepthMeters = 0.0f;
  telemetryAcc[0] = 0.0f; telemetryAcc[1] = 0.0f; telemetryAcc[2] = 0.0f;
  telemetryGyro[0] = 0.0f; telemetryGyro[1] = 0.0f; telemetryGyro[2] = 0.0f;
  telemetryAngle[0] = 0.0f; telemetryAngle[1] = 0.0f; telemetryAngle[2] = 0.0f;
  telemetryTempIn = 0.0f;

  if (depthReady) {
    if (depthSensor.update()) {
      telemetryDepthMeters = depthSensor.depthMeters();
    }
  }

  if (mpuReady) {
    mpu.update();

    telemetryAcc[0] = gToMps2(mpu.accX());
    telemetryAcc[1] = gToMps2(mpu.accY());
    telemetryAcc[2] = gToMps2(mpu.accZ());

    telemetryGyro[0] = degToRad(mpu.gyroX());
    telemetryGyro[1] = degToRad(mpu.gyroY());
    telemetryGyro[2] = degToRad(mpu.gyroZ());

    telemetryAngle[0] = degToRad(mpu.roll());
    telemetryAngle[1] = degToRad(mpu.pitch());
    telemetryAngle[2] = degToRad(mpu.yaw());

    telemetryTempIn = mpu.temperature();
  }
}

static void publishTelemetry() {
  StaticJsonDocument<TELEMETRY_JSON_CAPACITY> out;

  out["type"] = "sensors";
  JsonObject data = out["data"].to<JsonObject>();

  data["depth"] = telemetryDepthMeters;

  JsonObject mpuObj = data["mpu"].to<JsonObject>();

  JsonArray acc = mpuObj["acc"].to<JsonArray>();
  acc.add(telemetryAcc[0]);
  acc.add(telemetryAcc[1]);
  acc.add(telemetryAcc[2]);

  JsonArray gyro = mpuObj["gyro"].to<JsonArray>();
  gyro.add(telemetryGyro[0]);
  gyro.add(telemetryGyro[1]);
  gyro.add(telemetryGyro[2]);

  JsonArray angle = mpuObj["angle"].to<JsonArray>();
  angle.add(telemetryAngle[0]);
  angle.add(telemetryAngle[1]);
  angle.add(telemetryAngle[2]);

  mpuObj["temp_in"] = telemetryTempIn;

  serializeJson(out, Serial);
  Serial.println();
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

static void beginServos() {
  positionalServo1.begin();
  positionalServo2.begin();
  continuousServo1.begin();
  continuousServo2.begin();
  servosReady = true;
  moveServosToSafeHome();
}

static void beginSensors() {
  Wire.begin();

  mpuReady = mpu.begin();
  if (mpuReady) {
    delay(50);
    mpu.calibrateGyro(100);
  }

  depthReady = depthSensor.begin();
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, LOW);

  resetCommandState(commandState);
  rxIndex = 0;
  rxBuffer[0] = '\0';

  beginSensors();
  beginThrusters();
  beginServos();

  applyFailsafe();
  updateTelemetryCache();

  const unsigned long now = millis();
  lastCommandMs   = now;
  lastTelemetryMs = now;
  lastServoStepMs = now;
}

void loop() {
  processIncomingSerial();

  const unsigned long now = millis();

  if ((now - lastCommandMs) > COMMAND_TIMEOUT_MS) {
    applyFailsafe();
    lastCommandMs = now;
  } else {
    applyCommandState();
    updatePositionalServos(commandState);
  }

  if ((now - lastTelemetryMs) >= TELEMETRY_INTERVAL_MS) {
    lastTelemetryMs = now;
    updateTelemetryCache();
    publishTelemetry();
  }
}