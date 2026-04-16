#include <Arduino.h>
#include "servo.h"

// Use your module pin definition
const uint8_t SERVO_PIN = ServoConfig::POSITIONAL_SERVO_1_PIN; // 32

// Create positional servo (full range)
PositionalServo servo(
  SERVO_PIN,
  100.0f,   // min angle
  180.0f, // max angle
  90.0f   // home
);

void setup()
{
  Serial.begin(460800);
  delay(1000);

  Serial.println("=== Module Servo Serial Test ===");

  servo.begin();       // attaches + moves to home
  servo.setAngle(90);  // start centered

  Serial.println("Enter angle (0 - 180):");
}

void loop()
{
  if (Serial.available())
  {
    String input = Serial.readStringUntil('\n');
    input.trim();

    float angle = input.toFloat();

    // Send directly — module will clamp internally
    servo.setAngle(angle);

    Serial.print("Set angle: ");
    Serial.println(angle);
  }
}
