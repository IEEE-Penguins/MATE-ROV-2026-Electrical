#include <Arduino.h>
#include "servo.h"

// ======================================================
// Test objects
// ======================================================
PositionalServo cameraTilt(
    ServoConfig::POSITIONAL_SERVO_TEST_PIN,
    0.0f,
    180.0f,
    90.0f
);

ContinuousServo roller(
    ServoConfig::CONTINUOUS_SERVO_TEST_PIN,
    ServoConfig::CONTINUOUS_MIN_PULSE_US,
    ServoConfig::CONTINUOUS_NEUTRAL_PULSE_US,
    ServoConfig::CONTINUOUS_MAX_PULSE_US
);

// ======================================================
// Serial helpers
// ======================================================
void printDivider()
{
    Serial.println("--------------------------------------------------");
}

void printPositionalStatus(const char* label)
{
    Serial.print("[POSITIONAL] ");
    Serial.print(label);
    Serial.print(" | pin=");
    Serial.print(cameraTilt.getPin());
    Serial.print(" | angle=");
    Serial.println(cameraTilt.getAngle());
}

void printContinuousStatus(const char* label)
{
    Serial.print("[CONTINUOUS] ");
    Serial.print(label);
    Serial.print(" | pin=");
    Serial.print(roller.getPin());
    Serial.print(" | speed=");
    Serial.print(roller.getSpeed(), 2);
    Serial.print(" | pulse_us=");
    Serial.println(roller.getLastPulseUs());
}

// ======================================================
// Positional servo test
// ======================================================
void testPositionalServo()
{
    printDivider();
    Serial.println("Testing positional servo...");

    cameraTilt.moveHome();
    printPositionalStatus("Home");
    delay(1500);

    cameraTilt.setAngle(0.0f);
    printPositionalStatus("Move to 0 deg");
    delay(1500);

    cameraTilt.setAngle(180.0f);
    printPositionalStatus("Move to 180 deg");
    delay(1500);

    cameraTilt.setAngle(90.0f);
    printPositionalStatus("Move to 90 deg");
    delay(1500);

    cameraTilt.stepBy(ServoConfig::POSITIONAL_STEP_DEG);
    printPositionalStatus("Step +10 deg");
    delay(1000);

    cameraTilt.stepBy(-2.0f * ServoConfig::POSITIONAL_STEP_DEG);
    printPositionalStatus("Step -20 deg");
    delay(1000);

    cameraTilt.moveHome();
    printPositionalStatus("Return home");
    delay(1500);
}

// ======================================================
// Continuous servo test
// ======================================================
void testContinuousServo()
{
    printDivider();
    Serial.println("Testing continuous servo...");

    roller.stop();
    printContinuousStatus("Stop");
    delay(1500);

    roller.setSpeed(0.30f);
    printContinuousStatus("Forward 30%");
    delay(2000);

    roller.setSpeed(0.60f);
    printContinuousStatus("Forward 60%");
    delay(2000);

    roller.stop();
    printContinuousStatus("Stop");
    delay(1500);

    roller.setSpeed(-0.30f);
    printContinuousStatus("Reverse 30%");
    delay(2000);

    roller.setSpeed(-0.60f);
    printContinuousStatus("Reverse 60%");
    delay(2000);

    roller.stop();
    printContinuousStatus("Final stop");
    delay(1500);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("Servo module test starting...");

    cameraTilt.begin();
    roller.begin();

    Serial.println("Both servo objects initialized.");
}

void loop()
{
    testPositionalServo();
    testContinuousServo();
}