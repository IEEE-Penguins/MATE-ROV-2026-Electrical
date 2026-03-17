#include "servo.h"

// =====================================================
// Dual Positional Servo Unit Test
//
// What this test does:
// 1) Initializes two positional servos
// 2) Continuously sweeps both servos within their allowed ranges
//
// Servo ranges used in this test:
// - Servo 1: 116 to 180 degrees
// - Servo 2:   0 to 180 degrees
// =====================================================

// =====================================================
// User-adjustable test settings
// =====================================================

static const uint16_t SERVO_STEP_INTERVAL_MS = 25;
static const float SERVO_STEP_DEG = 1.0f;

// Use the servo module test pins by default.
// Change these if your actual wiring is different.
static const uint8_t SERVO_1_PIN = ServoConfig::POSITIONAL_SERVO_TEST_PIN;   // GPIO 32
static const uint8_t SERVO_2_PIN = ServoConfig::CONTINUOUS_SERVO_TEST_PIN;   // GPIO 33, reused here as positional

// =====================================================
// Positional servo objects
// =====================================================

// Servo 1: limited sweep from 116 -> 180 degrees
PositionalServo servo1(
  SERVO_1_PIN,
  116.0f,   // minimum allowed angle
  180.0f,   // maximum allowed angle
  116.0f    // home/start angle
);

// Servo 2: full sweep from 0 -> 180 degrees
PositionalServo servo2(
  SERVO_2_PIN,
  0.0f,     // minimum allowed angle
  180.0f,   // maximum allowed angle
  0.0f      // home/start angle
);

// =====================================================
// Sweep state
// =====================================================

float servo1Angle = 116.0f;
int servo1Dir = 1;

float servo2Angle = 0.0f;
int servo2Dir = 1;

unsigned long lastServoUpdateMs = 0;

// =====================================================
// Servo sweep logic
// =====================================================

void updateServoSweep()
{
  unsigned long now = millis();
  if (now - lastServoUpdateMs < SERVO_STEP_INTERVAL_MS)
  {
    return;
  }
  lastServoUpdateMs = now;

  // Servo 1 sweep: 116 -> 180 -> 116
  servo1Angle += servo1Dir * SERVO_STEP_DEG;

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

  // Servo 2 sweep: 0 -> 180 -> 0
  servo2Angle += servo2Dir * SERVO_STEP_DEG;

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

// =====================================================
// Arduino setup / loop
// =====================================================

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== Dual Positional Servo Unit Test ===");

  Serial.println("Initializing positional servos...");
  servo1.begin();
  servo2.begin();

  // Set initial servo positions
  servo1.setAngle(116.0f);
  servo2.setAngle(0.0f);

  delay(1000);

  Serial.println("Starting servo sweeps...");
  Serial.println("Servo 1 range: 116 -> 180 degrees");
  Serial.println("Servo 2 range: 0 -> 180 degrees");
}

void loop()
{
  updateServoSweep();
}