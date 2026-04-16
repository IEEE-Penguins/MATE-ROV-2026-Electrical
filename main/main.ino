#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>

#include "thruster.h"
#include "servo.h"
#include "mpu.h"
#include "depth.h"

// =====================================================
// Serial / protocol
// =====================================================
static const unsigned long SERIAL_BAUD_RATE = 460800;
static const size_t RX_BUFFER_SIZE = 160;
static const size_t TELEMETRY_JSON_CAPACITY = 320;

// Incoming control format (one line, newline-terminated):
// E,e1,e2,e3,e4,e5,e6,S,s1,s2,s3,s4,L,l1,l2
//
// Notes:
// - Thrusters parse only the E section values.
// - Servos parse only the S section values.
// - Lights parse only the L section values.
// - Outgoing telemetry remains JSON.

// =====================================================
// Timing
// =====================================================
static const unsigned long COMMAND_TIMEOUT_MS = 3000;
static const unsigned long TELEMETRY_INTERVAL_MS = 100;

// If true, servos keep their last valid command when command stream times out.
// Thrusters and light still go to safe state on timeout.
static const bool HOLD_SERVO_ON_TIMEOUT = true;

// =====================================================
// I2C
// =====================================================
// Set USE_EXPLICIT_I2C_PINS to true only if your ESP32 board
// needs manual SDA/SCL selection.
static const bool USE_EXPLICIT_I2C_PINS = false;
static const int I2C_SDA_PIN = 21;
static const int I2C_SCL_PIN = 22;

// =====================================================
// Lights
// =====================================================
// User confirmed physical light output on GPIO 12.
// Since command format still carries lights[2], this main turns the
// single physical light ON if either L value is non-zero.
static const uint8_t LIGHT_PIN = 12;

// =====================================================
// Servo limits
// =====================================================
static const float SERVO1_MIN_DEG = 116.0f; // calibrated safe minimum
static const float SERVO1_MAX_DEG = 180.0f;
static const float SERVO1_HOME_DEG = 116.0f;

static const float SERVO2_MIN_DEG = 0.0f;
static const float SERVO2_MAX_DEG = 180.0f;
static const float SERVO2_HOME_DEG = 60.0f;

// Positional camera servo slew limiting (servo1 + servo2 only)
// Move toward target in small steps for smoother motion and lower mechanical shock.
static const float SERVO_POS_STEP_DEG = 1.0f;
static const unsigned long SERVO_POS_STEP_INTERVAL_MS = 20;

// Continuous servo tuning
// Use wide pulse range similar to standalone Servo.write(0..180) testing.
// This often gives noticeably higher top speed on many continuous servos.
// Incoming control is already -1 or 1, so no extra gain is needed.
static const float SERVO_CONT_COMMAND_GAIN = 1.0f;
static const int SERVO_CONT_MIN_PULSE_US = 544;
static const int SERVO_CONT_NEUTRAL_PULSE_US = 1500;
static const int SERVO_CONT_MAX_PULSE_US = 2400;

// =====================================================
// Reordering hooks
// =====================================================
// To change logical command order later, edit ONLY these mappings.
// Example: if GUI esc[0] should drive physical thruster 4, set first
// entry to 3.
static const uint8_t THRUSTER_CMD_TO_PHYSICAL[THRUSTER_COUNT] = {0, 1, 2, 3, 4, 5}; // >>>>> Thrusters order
static const bool THRUSTER_INVERTED[THRUSTER_COUNT] = {false, false, false, false, false, false};

static const uint8_t SERVO_POS1_CMD_INDEX = 0;
static const uint8_t SERVO_POS2_CMD_INDEX = 1;
static const uint8_t SERVO_CONT1_CMD_INDEX = 2;
static const uint8_t SERVO_CONT2_CMD_INDEX = 3;

// =====================================================
// Modules
// =====================================================
MPU6050 mpu(Wire);
DepthSensor depthSensor(Wire);

ThrusterConfig thrusterCfg1(THRUSTER_1_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, THRUSTER_INVERTED[0]);
ThrusterConfig thrusterCfg2(THRUSTER_2_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, THRUSTER_INVERTED[1]);
ThrusterConfig thrusterCfg3(THRUSTER_3_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, THRUSTER_INVERTED[2]);
ThrusterConfig thrusterCfg4(THRUSTER_4_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, THRUSTER_INVERTED[3]);
ThrusterConfig thrusterCfg5(THRUSTER_5_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, THRUSTER_INVERTED[4]);
ThrusterConfig thrusterCfg6(THRUSTER_6_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, THRUSTER_INVERTED[5]);

Thruster gThruster1(thrusterCfg1);
Thruster gThruster2(thrusterCfg2);
Thruster gThruster3(thrusterCfg3);
Thruster gThruster4(thrusterCfg4);
Thruster gThruster5(thrusterCfg5);
Thruster gThruster6(thrusterCfg6);

Thruster *physicalThrusterArray[THRUSTER_COUNT] = {
    &gThruster1,
    &gThruster2,
    &gThruster3,
    &gThruster4,
    &gThruster5,
    &gThruster6};

Thruster *thrusterArray[THRUSTER_COUNT] = {
    physicalThrusterArray[THRUSTER_CMD_TO_PHYSICAL[0]],
    physicalThrusterArray[THRUSTER_CMD_TO_PHYSICAL[1]],
    physicalThrusterArray[THRUSTER_CMD_TO_PHYSICAL[2]],
    physicalThrusterArray[THRUSTER_CMD_TO_PHYSICAL[3]],
    physicalThrusterArray[THRUSTER_CMD_TO_PHYSICAL[4]],
    physicalThrusterArray[THRUSTER_CMD_TO_PHYSICAL[5]]};

Thrusters thrusters(thrusterArray, THRUSTER_COUNT);
bool thrustersReady = false;

PositionalServo servo1(
    ServoConfig::POSITIONAL_SERVO_1_PIN,
    SERVO1_MIN_DEG,
    SERVO1_MAX_DEG,
    SERVO1_HOME_DEG,
    ServoConfig::DEFAULT_MIN_PULSE_US,
    ServoConfig::DEFAULT_MAX_PULSE_US);

PositionalServo servo2(
    ServoConfig::POSITIONAL_SERVO_2_PIN,
    SERVO2_MIN_DEG,
    SERVO2_MAX_DEG,
    SERVO2_HOME_DEG,
    ServoConfig::DEFAULT_MIN_PULSE_US,
    ServoConfig::DEFAULT_MAX_PULSE_US);

ContinuousServo servo3(
    ServoConfig::CONTINUOUS_SERVO_1_PIN,
    SERVO_CONT_MIN_PULSE_US,
    SERVO_CONT_NEUTRAL_PULSE_US,
    SERVO_CONT_MAX_PULSE_US);

ContinuousServo servo4(
    ServoConfig::CONTINUOUS_SERVO_2_PIN,
    SERVO_CONT_MIN_PULSE_US,
    SERVO_CONT_NEUTRAL_PULSE_US,
    SERVO_CONT_MAX_PULSE_US);

bool servosReady = false;
bool mpuReady = false;
bool depthReady = false;

// =====================================================
// Command state
// =====================================================
struct CommandState
{
  float esc[THRUSTER_COUNT];
  float servo[4];
  int lights[2];
  bool valid;
};

CommandState commandState;

// =====================================================
// Runtime state
// =====================================================
char rxBuffer[RX_BUFFER_SIZE];
size_t rxIndex = 0;
unsigned long lastCommandMs = 0;
unsigned long lastTelemetryMs = 0;
unsigned long lastPositionalServoStepMs = 0;

// =====================================================
// Helpers
// =====================================================

static float mapNormalizedToAngle(float command, float minDeg, float maxDeg)
{
  command = clampNormalized(command);
  const float t = (command + 1.0f) * 0.5f;
  return minDeg + t * (maxDeg - minDeg);
}

static float clampNormalized(float value)
{
  if (value < -1.0f)
    return -1.0f;
  if (value > 1.0f)
    return 1.0f;
  return value;
}

static float clampFloat(float value, float minValue, float maxValue)
{
  if (value < minValue)
    return minValue;
  if (value > maxValue)
    return maxValue;
  return value;
}

static int clampLightValue(int value)
{
  return (value != 0) ? 1 : 0;
}

static void resetCommandState(CommandState &state)
{
  for (uint8_t i = 0; i < THRUSTER_COUNT; ++i)
  {
    state.esc[i] = 0.0f;
  }

  for (uint8_t i = 0; i < 4; ++i)
  {
    state.servo[i] = 0.0f;
  }

  state.lights[0] = 0;
  state.lights[1] = 0;
  state.valid = false;
}

static void stopThrusters()
{
  if (thrustersReady)
  {
    thrusters.stopAll();
  }
}

static void moveToSafeServoState()
{
  if (!servosReady)
  {
    return;
  }

  // servo1.moveHome();
  // servo2.moveHome();
  servo3.stop();
  servo4.stop();
}

static void setLights(const CommandState &state)
{
  const bool lightOn = (state.lights[0] != 0) || (state.lights[1] != 0);
  digitalWrite(LIGHT_PIN, lightOn ? HIGH : LOW);
}

static void applyThrusters(const CommandState &state)
{
  if (!thrustersReady || !state.valid)
  {
    return;
  }

  float commands[THRUSTER_COUNT];
  for (uint8_t i = 0; i < THRUSTER_COUNT; ++i)
  {
    commands[i] = clampNormalized(state.esc[i]);
  }

  thrusters.setAll(commands, THRUSTER_COUNT);
}

static void applyServos(const CommandState &state)
{
  if (!servosReady || !state.valid)
  {
    return;
  }

  const float s1Angle = mapNormalizedToAngle(state.servo[SERVO_POS1_CMD_INDEX], SERVO1_MIN_DEG, SERVO1_MAX_DEG);
  const float s2Angle = mapNormalizedToAngle(state.servo[SERVO_POS2_CMD_INDEX], SERVO2_MIN_DEG, SERVO2_MAX_DEG);
  const float s3Speed = clampNormalized(state.servo[SERVO_CONT1_CMD_INDEX] * SERVO_CONT_COMMAND_GAIN);
  const float s4Speed = clampNormalized(state.servo[SERVO_CONT2_CMD_INDEX] * SERVO_CONT_COMMAND_GAIN);

  const unsigned long now = millis();
  if ((now - lastPositionalServoStepMs) >= SERVO_POS_STEP_INTERVAL_MS)
  {
    lastPositionalServoStepMs = now;

    const float s1Delta = clampFloat(s1Angle - servo1.getAngle(), -SERVO_POS_STEP_DEG, SERVO_POS_STEP_DEG);
    const float s2Delta = clampFloat(s2Angle - servo2.getAngle(), -SERVO_POS_STEP_DEG, SERVO_POS_STEP_DEG);

    servo1.stepBy(s1Delta);
    servo2.stepBy(s2Delta);
  }

  servo3.setSpeed(s3Speed);
  servo4.setSpeed(s4Speed);
}

static void applyCommand(const CommandState &state)
{
  applyThrusters(state);
  applyServos(state);
  setLights(state);
}

static void applyFailsafe()
{
  resetCommandState(commandState);
  commandState.valid = true;
  stopThrusters();
  moveToSafeServoState();
  digitalWrite(LIGHT_PIN, LOW);
}

static void applyTimeoutSafetyBehavior()
{
  stopThrusters();
  digitalWrite(LIGHT_PIN, LOW);

  if (!HOLD_SERVO_ON_TIMEOUT)
  {
    moveToSafeServoState();
  }
}

// Parse incoming control line:
// E,e1,e2,e3,e4,e5,e6,S,s1,s2,s3,s4,L,l1,l2
//
// Required behavior:
// - E section must exist and contain 6 values
// - S section must exist and contain 4 values
// - L section must exist and contain 2 values
static bool parseCommandFrame(char *line, CommandState &outState)
{
  char *token = strtok(line, ",");
  if (token == nullptr || strcmp(token, "E") != 0)
  {
    return false;
  }

  CommandState parsed;
  resetCommandState(parsed);

  for (uint8_t i = 0; i < THRUSTER_COUNT; ++i)
  {
    token = strtok(nullptr, ",");
    if (token == nullptr)
    {
      return false;
    }
    parsed.esc[i] = clampNormalized(static_cast<float>(atof(token)));
  }

  token = strtok(nullptr, ",");
  if (token == nullptr || strcmp(token, "S") != 0)
  {
    return false;
  }

  for (uint8_t i = 0; i < 4; ++i)
  {
    token = strtok(nullptr, ",");
    if (token == nullptr)
    {
      return false;
    }
    parsed.servo[i] = static_cast<float>(atof(token));
  }

  token = strtok(nullptr, ",");
  if (token == nullptr || strcmp(token, "L") != 0)
  {
    return false;
  }

  for (uint8_t i = 0; i < 2; ++i)
  {
    token = strtok(nullptr, ",");
    if (token == nullptr)
    {
      return false;
    }
    parsed.lights[i] = clampLightValue(atoi(token));
  }

  parsed.valid = true;
  outState = parsed;
  return true;
}

static void processIncomingSerial()
{
  while (Serial.available() > 0)
  {
    const char c = static_cast<char>(Serial.read());

    if (c == '\r')
    {
      continue;
    }

    if (c == '\n')
    {
      rxBuffer[rxIndex] = '\0';

      if (rxIndex > 0)
      {
        CommandState parsed;
        if (parseCommandFrame(rxBuffer, parsed))
        {
          commandState = parsed;
          lastCommandMs = millis();
        }
      }

      rxIndex = 0;
      rxBuffer[0] = '\0';
      continue;
    }

    if (rxIndex < (RX_BUFFER_SIZE - 1))
    {
      rxBuffer[rxIndex++] = c;
    }
    else
    {
      rxIndex = 0;
      rxBuffer[0] = '\0';
    }
  }
}

static void publishTelemetry(bool commandAlive)
{
  StaticJsonDocument<TELEMETRY_JSON_CAPACITY> out;

  out["type"] = "sensors";
  JsonObject data = out["data"].to<JsonObject>();

  data["depth"] = depthReady ? depthSensor.depthMeters() : 0.0f;
  data["depth_status"] = DepthSensor::statusToString(depthSensor.status());

  JsonObject mpuObj = data["mpu"].to<JsonObject>();

  JsonArray acc = mpuObj["acc"].to<JsonArray>();
  if (mpuReady)
  {
    acc.add(mpu.accX() * 9.80665f);
    acc.add(mpu.accY() * 9.80665f);
    acc.add(mpu.accZ() * 9.80665f);
  }
  else
  {
    acc.add(0.0f);
    acc.add(0.0f);
    acc.add(0.0f);
  }

  JsonArray gyro = mpuObj["gyro"].to<JsonArray>();
  if (mpuReady)
  {
    gyro.add(mpu.gyroX() * DEG_TO_RAD);
    gyro.add(mpu.gyroY() * DEG_TO_RAD);
    gyro.add(mpu.gyroZ() * DEG_TO_RAD);
  }
  else
  {
    gyro.add(0.0f);
    gyro.add(0.0f);
    gyro.add(0.0f);
  }

  JsonArray angle = mpuObj["angle"].to<JsonArray>();
  if (mpuReady)
  {
    angle.add(mpu.roll() * DEG_TO_RAD);
    angle.add(mpu.pitch() * DEG_TO_RAD);
    angle.add(mpu.yaw() * DEG_TO_RAD);
  }
  else
  {
    angle.add(0.0f);
    angle.add(0.0f);
    angle.add(0.0f);
  }

  mpuObj["temp_in"] = mpuReady ? mpu.temperature() : 0.0f;
  mpuObj["ready"] = mpuReady;

  data["command_alive"] = commandAlive;

  serializeJson(out, Serial);
  Serial.println();
}

static void beginI2C()
{
  if (USE_EXPLICIT_I2C_PINS)
  {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  }
  else
  {
    Wire.begin();
  }
}

static void beginThrusters()
{
  thrusters.beginAll();
  thrusters.stopAll();
  thrustersReady = true;
}

static void beginServos()
{
  servo1.begin();
  servo2.begin();
  servo3.begin();
  servo4.begin();
  servosReady = true;
  moveToSafeServoState();
}

static void beginLights()
{
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, LOW);
}

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);

  resetCommandState(commandState);
  rxIndex = 0;
  rxBuffer[0] = '\0';

  beginLights();
  beginI2C();

  mpuReady = mpu.begin();
  depthReady = depthSensor.begin();

  beginThrusters();
  beginServos();
  applyFailsafe();

  lastCommandMs = millis();
  lastTelemetryMs = millis();
  lastPositionalServoStepMs = millis();
}

void loop()
{
  processIncomingSerial();

  if (mpuReady)
  {
    mpu.update();
  }

  if (depthReady)
  {
    depthSensor.update();
  }

  const unsigned long now = millis();
  const bool commandAlive = ((now - lastCommandMs) <= COMMAND_TIMEOUT_MS);

  if (commandAlive)
  {
    applyCommand(commandState);
  }
  else
  {
    applyTimeoutSafetyBehavior();
  }

  if ((now - lastTelemetryMs) >= TELEMETRY_INTERVAL_MS)
  {
    lastTelemetryMs = now;
    publishTelemetry(commandAlive);
  }
}
