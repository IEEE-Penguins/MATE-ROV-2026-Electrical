#include "thruster.h"
#include "servo.h"

// =====================================================
// Thruster + Dual Positional Servo Unit Test
//
// What this test does:
// 1) Arms and runs all thrusters at the same time
// 2) Sweeps TWO POSITIONAL SERVOS continuously
//
// Servo ranges used in this test:
// - Servo 1: sweeps from 116 to 180 degrees
// - Servo 2: sweeps from   0 to 180 degrees
// =====================================================

// =====================================================
// User-adjustable test settings
// =====================================================

// Safe low-speed thruster command for bench testing
static const float THRUSTER_TEST_COMMAND = 0.20f;

// Sweep timing
static const uint16_t SERVO_STEP_INTERVAL_MS = 25;
static const float SERVO_STEP_DEG = 1.0f;

// Use the servo module test pins by default.
// Change these if your actual wiring is different.
static const uint8_t SERVO_1_PIN = ServoConfig::POSITIONAL_SERVO_TEST_PIN;
static const uint8_t SERVO_2_PIN = ServoConfig::CONTINUOUS_SERVO_TEST_PIN; // reused here as a normal positional servo pin

// =====================================================
// Thruster objects
// =====================================================

ThrusterConfig t1Cfg(THRUSTER_1_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, false);
ThrusterConfig t2Cfg(THRUSTER_2_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, false);
ThrusterConfig t3Cfg(THRUSTER_3_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, false);
ThrusterConfig t4Cfg(THRUSTER_4_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, false);
ThrusterConfig t5Cfg(THRUSTER_5_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, false);
ThrusterConfig t6Cfg(THRUSTER_6_PIN, ESC_US_MIN, ESC_US_NEUTRAL, ESC_US_MAX, ESC_PWM_HZ, THRUSTER_DEFAULT_DEADBAND, false);

Thruster thruster1(t1Cfg);
Thruster thruster2(t2Cfg);
Thruster thruster3(t3Cfg);
Thruster thruster4(t4Cfg);
Thruster thruster5(t5Cfg);
Thruster thruster6(t6Cfg);

Thruster* thrusterList[] = {
  &thruster1,
  &thruster2,
  &thruster3,
  &thruster4,
  &thruster5,
  &thruster6
};

Thrusters thrusters(thrusterList, THRUSTER_COUNT);

// =====================================================
// Positional servo objects
// =====================================================

// SERVO 1:
// Positional servo sweeping only in the range 116 -> 180 degrees
PositionalServo servo1(
  SERVO_1_PIN,
  116.0f,   // minimum allowed angle for servo 1
  180.0f,   // maximum allowed angle for servo 1
  116.0f    // home/start angle for servo 1
);

// SERVO 2:
// Positional servo sweeping in the full range 0 -> 180 degrees
PositionalServo servo2(
  SERVO_2_PIN,
  0.0f,     // minimum allowed angle for servo 2
  180.0f,   // maximum allowed angle for servo 2
  0.0f      // home/start angle for servo 2
);

// =====================================================
// Sweep state
// =====================================================

// Servo 1 current angle and direction
// Range: 116 <-> 180
float servo1Angle = 116.0f;
int servo1Dir = 1;

// Servo 2 current angle and direction
// Range: 0 <-> 180
float servo2Angle = 0.0f;
int servo2Dir = 1;

unsigned long lastServoUpdateMs = 0;

// =====================================================
// Helpers
// =====================================================

void runAllThrusters(float command)
{
  float commands[THRUSTER_COUNT] = {
    command, command, command, command, command, command
  };
  thrusters.setAll(commands, THRUSTER_COUNT);
}

void updateServoSweep()
{
  unsigned long now = millis();
  if (now - lastServoUpdateMs < SERVO_STEP_INTERVAL_MS)
  {
    return;
  }
  lastServoUpdateMs = now;

  // -------------------------------------------------
  // Servo 1 sweep: 116 -> 180 -> 116
  // -------------------------------------------------
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

  // -------------------------------------------------
  // Servo 2 sweep: 0 -> 180 -> 0
  // -------------------------------------------------
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

  Serial.println("\n=== Thruster + Positional Servo Unit Test ===");

  Serial.println("Initializing thrusters...");
  thrusters.beginAll();

  Serial.println("Arming all ESCs...");
  thrusters.armAll(ESC_ARM_DELAY_MS);

  Serial.println("Initializing positional servos...");
  servo1.begin();
  servo2.begin();

  // Set initial servo positions
  // Servo 1 starts at 116 degrees
  // Servo 2 starts at 0 degrees
  servo1.setAngle(116.0f);
  servo2.setAngle(0.0f);

  delay(1000);

  Serial.println("Starting all thrusters...");
  runAllThrusters(THRUSTER_TEST_COMMAND);

  Serial.println("Starting servo sweeps...");
  Serial.println("Servo 1 range: 116 -> 180 degrees");
  Serial.println("Servo 2 range: 0 -> 180 degrees");
}

void loop()
{
  // Keep all thrusters running together
  runAllThrusters(THRUSTER_TEST_COMMAND);

  // Continuously sweep both POSITIONAL servos
  updateServoSweep();
}