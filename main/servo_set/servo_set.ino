#include <Arduino.h>
#include "servo.h"

static const unsigned long SERIAL_BAUD_RATE = 460800;

// =====================================================
// Positional servo ranges and defaults
// =====================================================
static const float SERVO1_MIN_DEG = 10.0f;
static const float SERVO1_MAX_DEG = 2000.0f;
static const float SERVO1_DEFAULT_DEG = 10.0f;

static const float SERVO2_MIN_DEG = 0.0f;
static const float SERVO2_MAX_DEG = 2000.0f;
static const float SERVO2_DEFAULT_DEG = 10.0f;

// =====================================================
// Startup timing
// =====================================================
static const unsigned long STARTUP_HOLD_MS = 900;

// =====================================================
// Servo objects (only positional servos)
// =====================================================
PositionalServo servo1(
    ServoConfig::POSITIONAL_SERVO_1_PIN,
    SERVO1_MIN_DEG,
    SERVO1_MAX_DEG,
    SERVO1_DEFAULT_DEG,
    ServoConfig::DEFAULT_MIN_PULSE_US,
    ServoConfig::DEFAULT_MAX_PULSE_US);

PositionalServo servo2(
    ServoConfig::POSITIONAL_SERVO_2_PIN,
    SERVO2_MIN_DEG,
    SERVO2_MAX_DEG,
    SERVO2_DEFAULT_DEG,
    ServoConfig::DEFAULT_MIN_PULSE_US,
    ServoConfig::DEFAULT_MAX_PULSE_US);

void printHelp()
{
    Serial.println("Commands:");
    Serial.println("  s1 <angle>      -> set servo1 only");
    Serial.println("  s2 <angle>      -> set servo2 only");
    Serial.println("  <a1> <a2>       -> set servo1 and servo2 together");
    Serial.println("  home            -> move both to default position");
    Serial.println("  help            -> print this help");
}

void moveToDefault()
{
    servo1.setAngle(SERVO1_DEFAULT_DEG);
    servo2.setAngle(SERVO2_DEFAULT_DEG);
}

void runStartupLimitMove()
{
    // Move to minimum limits
    servo1.setAngle(SERVO1_MIN_DEG);
    servo2.setAngle(SERVO2_MIN_DEG);
    delay(STARTUP_HOLD_MS);

    // // Move to maximum limits
    // servo1.setAngle(SERVO1_MAX_DEG);
    // servo2.setAngle(SERVO2_MAX_DEG);
    // delay(STARTUP_HOLD_MS);

    // Return to default position
    moveToDefault();
    delay(STARTUP_HOLD_MS);
}

void handleSerialCommand(const String &line)
{
    if (line.length() == 0)
    {
        return;
    }

    if (line.equalsIgnoreCase("help"))
    {
        printHelp();
        return;
    }

    if (line.equalsIgnoreCase("home"))
    {
        moveToDefault();
        Serial.println("Moved both servos to default positions.");
        return;
    }

    float a1 = 0.0f;
    float a2 = 0.0f;
    if (sscanf(line.c_str(), "%f %f", &a1, &a2) == 2)
    {
        servo1.setAngle(a1);
        servo2.setAngle(a2);

        Serial.print("Set servo1=");
        Serial.print(servo1.getAngle());
        Serial.print(" servo2=");
        Serial.println(servo2.getAngle());
        return;
    }

    char name[8] = {0};
    float angle = 0.0f;
    if (sscanf(line.c_str(), "%7s %f", name, &angle) == 2)
    {
        String which(name);

        if (which.equalsIgnoreCase("s1"))
        {
            servo1.setAngle(angle);
            Serial.print("Set servo1=");
            Serial.println(servo1.getAngle());
            return;
        }

        if (which.equalsIgnoreCase("s2"))
        {
            servo2.setAngle(angle);
            Serial.print("Set servo2=");
            Serial.println(servo2.getAngle());
            return;
        }
    }

    Serial.println("Invalid command. Type 'help'.");
}

void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);

    servo1.begin();
    servo2.begin();

    Serial.println("=== Servo Set Mode (Positional only) ===");
    Serial.println("Startup: min -> max -> default");
    Serial.println("Waiting for serial commands...");

    runStartupLimitMove();
    printHelp();
}

void loop()
{
    if (Serial.available())
    {
        String line = Serial.readStringUntil('\n');
        line.trim();
        handleSerialCommand(line);
    }
}
