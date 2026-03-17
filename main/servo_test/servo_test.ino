#include "servo.h"

// =====================================================
// 4-Servo Unit Test
//
// What this test does:
// 1) Sweeps two positional servos within their angle limits
// 2) Sweeps two continuous servos within their speed limits
//
// Positional servo ranges:
// - Servo 1: 116 -> 180 degrees
// - Servo 2:   0 -> 180 degrees
//
// Continuous servo speed ranges:
// - Servo 3: -1.0 -> 1.0
// - Servo 4: -1.0 -> 1.0
// =====================================================

// =====================================================
// User-adjustable test settings
// =====================================================

static const uint16_t POSITIONAL_STEP_INTERVAL_MS = 25;
static const float POSITIONAL_STEP_DEG = 1.0f;

static const uint16_t CONTINUOUS_STEP_INTERVAL_MS = 40;
static const float CONTINUOUS_STEP = 0.05f;

// Pin map
// Assumes your restructured servo module uses these final pins:
// - positional 1 = 32
// - positional 2 = 33
// - continuous 1 = 16
// - continuous 2 = 17
static const uint8_t SERVO_1_PIN = 32;
static const uint8_t SERVO_2_PIN = 33;
static const uint8_t SERVO_3_PIN = 16;
static const uint8_t SERVO_4_PIN = 17;

// =====================================================
// Servo objects
// =====================================================

// Positional servo 1: 116 -> 180
PositionalServo servo1(
  SERVO_1_PIN,
  116.0f,
  180.0f,
  116.0f
);

// Positional servo 2: 0 -> 180
PositionalServo servo2(
  SERVO_2_PIN,
  0.0f,
  180.0f,
  0.0f
);

// Continuous servo 1: speed -1.0 -> 1.0
ContinuousServo servo3(SERVO_3_PIN);

// Continuous servo 2: speed -1.0 -> 1.0
ContinuousServo servo4(SERVO_4_PIN);

// =====================================================
// Sweep state
// =====================================================

// Positional servo state
float servo1Angle = 116.0f;
int servo1Dir = 1;

float servo2Angle = 0.0f;
int servo2Dir = 1;

// Continuous servo state
float servo3Speed = -1.0f;
int servo3Dir = 1;

float servo4Speed = -1.0f;
int servo4Dir = 1;

unsigned long lastPositionalUpdateMs = 0;
unsigned long lastContinuousUpdateMs = 0;

// =====================================================
// Sweep helpers
// =====================================================

void updatePositionalSweep()
{
  unsigned long now = millis();
  if (now - lastPositionalUpdateMs < POSITIONAL_STEP_INTERVAL_MS)
  {
    return;
  }
  lastPositionalUpdateMs = now;

  // Servo 1: 116 -> 180 -> 116
  servo1Angle += servo1Dir * POSITIONAL_STEP_DEG;

  if (servo1Angle >= 180.0f)
  {
    servo1Angle = 180.0f;
    servo1Dir = -1;
  }
  else if (servo1Angle <= 116.0f)
  {
    servo1Angle = 116.0f;
    servo1Dir = 1;
  }

  // Servo 2: 0 -> 180 -> 0
  servo2Angle += servo2Dir * POSITIONAL_STEP_DEG;

  if (servo2Angle >= 180.0f)
  {
    servo2Angle = 180.0f;
    servo2Dir = -1;
  }
  else if (servo2Angle <= 0.0f)
  {
    servo2Angle = 0.0f;
    servo2Dir = 1;
  }

  servo1.setAngle(servo1Angle);
  servo2.setAngle(servo2Angle);
}

void updateContinuousSweep()
{
  unsigned long now = millis();
  if (now - lastContinuousUpdateMs < CONTINUOUS_STEP_INTERVAL_MS)
  {
    return;
  }
  lastContinuousUpdateMs = now;

  // Servo 3: -1.0 -> 1.0 -> -1.0
  servo3Speed += servo3Dir * CONTINUOUS_STEP;

  if (servo3Speed >= 1.0f)
  {
    servo3Speed = 1.0f;
    servo3Dir = -1;
  }
  else if (servo3Speed <= -1.0f)
  {
    servo3Speed = -1.0f;
    servo3Dir = 1;
  }

  // Servo 4: -1.0 -> 1.0 -> -1.0
  servo4Speed += servo4Dir * CONTINUOUS_STEP;

  if (servo4Speed >= 1.0f)
  {
    servo4Speed = 1.0f;
    servo4Dir = -1;
  }
  else if (servo4Speed <= -1.0f)
  {
    servo4Speed = -1.0f;
    servo4Dir = 1;
  }

  servo3.setSpeed(servo3Speed);
  servo4.setSpeed(servo4Speed);
}

// =====================================================
// Arduino setup / loop
// =====================================================

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== 4-Servo Unit Test ===");

  Serial.println("Initializing servos...");
  servo1.begin();
  servo2.begin();
  servo3.begin();
  servo4.begin();

  // Initial positions / speeds
  servo1.setAngle(116.0f);
  servo2.setAngle(0.0f);
  servo3.setSpeed(-1.0f);
  servo4.setSpeed(-1.0f);

  delay(1000);

  Serial.println("Starting servo sweeps...");
  Serial.println("Servo 1 angle range: 116 -> 180 deg");
  Serial.println("Servo 2 angle range: 0 -> 180 deg");
  Serial.println("Servo 3 speed range: -1.0 -> 1.0");
  Serial.println("Servo 4 speed range: -1.0 -> 1.0");
}

void loop()
{
  updatePositionalSweep();
  updateContinuousSweep();
}