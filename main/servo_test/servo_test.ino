#include <Arduino.h>
#include "servo.h"

// =====================================================
// Dual Continuous Servo Unit Test
//
// What this test does:
// 1) Creates two ContinuousServo objects on pins 16 and 17
// 2) Sweeps both servos together from full reverse
//    to full forward and back again
//
// Sweep behavior:
// - speed: -1.0 -> 1.0 -> -1.0
// - both servos get the same command
// =====================================================

// =====================================================
// Pins
// =====================================================
static const uint8_t SERVO_1_PIN = 26;
static const uint8_t SERVO_2_PIN = 25;

// =====================================================
// Sweep settings
// =====================================================
static const uint16_t SERVO_STEP_INTERVAL_MS = 40;
static const float SERVO_STEP = 0.05f;

// =====================================================
// Servo objects
// =====================================================
ContinuousServo servo1(SERVO_1_PIN);
ContinuousServo servo2(SERVO_2_PIN);

// =====================================================
// Sweep state
// =====================================================
float currentSpeed = -1.0f;
int direction = 1;
unsigned long lastUpdateMs = 0;

// =====================================================
// Helpers
// =====================================================
void printStatus()
{
  Serial.print("Speed: ");
  Serial.print(currentSpeed, 2);
  Serial.print(" | Pulse1: ");
  Serial.print(servo1.getLastPulseUs());
  Serial.print(" | Pulse2: ");
  Serial.println(servo2.getLastPulseUs());
}

void updateSweep()
{
  unsigned long now = millis();
  if (now - lastUpdateMs < SERVO_STEP_INTERVAL_MS)
  {
    return;
  }
  lastUpdateMs = now;

  currentSpeed += direction * SERVO_STEP;

  if (currentSpeed >= 1.0f)
  {
    currentSpeed = 1.0f;
    direction = -1;
  }
  else if (currentSpeed <= -1.0f)
  {
    currentSpeed = -1.0f;
    direction = 1;
  }

  servo1.setSpeed(currentSpeed);
  servo2.setSpeed(currentSpeed);

  printStatus();
}

// =====================================================
// Arduino setup / loop
// =====================================================
void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=== Dual Continuous Servo Module Test ===");
  Serial.println("Pins: 16, 17");
  Serial.println("Sweep: -1.0 -> 1.0 -> -1.0");

  servo1.begin();
  servo2.begin();

  servo1.setSpeed(-1.0f);
  servo2.setSpeed(-1.0f);

  delay(1000);
}

void loop()
{
  updateSweep();
}