#include <Arduino.h>
#include "servo.h"

static const unsigned long SERIAL_BAUD_RATE = 460800;

// =====================================================
// Positional servo ranges
// =====================================================
static const float SERVO1_MIN_DEG = 116.0f;
static const float SERVO1_MAX_DEG = 180.0f;
static const float SERVO1_HOME_DEG = 116.0f;

static const float SERVO2_MIN_DEG = 0.0f;
static const float SERVO2_MAX_DEG = 180.0f;
static const float SERVO2_HOME_DEG = 90.0f;

// =====================================================
// Sweep settings
// =====================================================
static const unsigned long POSITIONAL_STEP_DELAY_MS = 20;
static const float POSITIONAL_STEP_DEG = 2.0f;

static const unsigned long CONTINUOUS_RUN_TIME_MS = 10000;
static const unsigned long CONTINUOUS_STOP_TIME_MS = 800;
static const float CONTINUOUS_TEST_SPEED = 0.9f;

// =====================================================
// Servo objects
// =====================================================
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
    ServoConfig::CONTINUOUS_MIN_PULSE_US,
    ServoConfig::CONTINUOUS_NEUTRAL_PULSE_US,
    ServoConfig::CONTINUOUS_MAX_PULSE_US);

ContinuousServo servo4(
    ServoConfig::CONTINUOUS_SERVO_2_PIN,
    ServoConfig::CONTINUOUS_MIN_PULSE_US,
    ServoConfig::CONTINUOUS_NEUTRAL_PULSE_US,
    ServoConfig::CONTINUOUS_MAX_PULSE_US);

// =====================================================
// Helpers
// =====================================================
void setPositionalToStart()
{
    servo1.setAngle(SERVO1_MIN_DEG);
    servo2.setAngle(SERVO2_MIN_DEG);
}

void setPositionalToEnd()
{
    servo1.setAngle(SERVO1_MAX_DEG);
    servo2.setAngle(SERVO2_MAX_DEG);
}

void sweepPositionalForward()
{
    float a1 = SERVO1_MIN_DEG;
    float a2 = SERVO2_MIN_DEG;

    while (a1 < SERVO1_MAX_DEG || a2 < SERVO2_MAX_DEG)
    {
        if (a1 < SERVO1_MAX_DEG)
        {
            a1 += POSITIONAL_STEP_DEG;
            if (a1 > SERVO1_MAX_DEG)
            {
                a1 = SERVO1_MAX_DEG;
            }
        }

        if (a2 < SERVO2_MAX_DEG)
        {
            a2 += POSITIONAL_STEP_DEG;
            if (a2 > SERVO2_MAX_DEG)
            {
                a2 = SERVO2_MAX_DEG;
            }
        }

        servo1.setAngle(a1);
        servo2.setAngle(a2);

        delay(POSITIONAL_STEP_DELAY_MS);
    }
}

void sweepPositionalBackward()
{
    float a1 = SERVO1_MAX_DEG;
    float a2 = SERVO2_MAX_DEG;

    while (a1 > SERVO1_MIN_DEG || a2 > SERVO2_MIN_DEG)
    {
        if (a1 > SERVO1_MIN_DEG)
        {
            a1 -= POSITIONAL_STEP_DEG;
            if (a1 < SERVO1_MIN_DEG)
            {
                a1 = SERVO1_MIN_DEG;
            }
        }

        if (a2 > SERVO2_MIN_DEG)
        {
            a2 -= POSITIONAL_STEP_DEG;
            if (a2 < SERVO2_MIN_DEG)
            {
                a2 = SERVO2_MIN_DEG;
            }
        }

        servo1.setAngle(a1);
        servo2.setAngle(a2);

        delay(POSITIONAL_STEP_DELAY_MS);
    }
}

void runContinuousForward()
{
    // servo3.setSpeed(CONTINUOUS_TEST_SPEED);
    servo4.setSpeed(CONTINUOUS_TEST_SPEED);
    delay(CONTINUOUS_RUN_TIME_MS);
}

void runContinuousReverse()
{
    // servo3.setSpeed(-CONTINUOUS_TEST_SPEED);
    servo4.setSpeed(-CONTINUOUS_TEST_SPEED);
    delay(CONTINUOUS_RUN_TIME_MS);
}

void stopContinuous()
{
    servo3.stop();
    servo4.stop();
    delay(CONTINUOUS_STOP_TIME_MS);
}

// =====================================================
// Setup / loop
// =====================================================
void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);

    Serial.println("=== All Servo Sweep Test ===");
    Serial.println("Servo1: 116 -> 180");
    Serial.println("Servo2: 0 -> 180");
    Serial.println("Servo3/4: continuous forward/reverse");

    servo1.begin();
    servo2.begin();
    servo3.begin();
    servo4.begin();

    setPositionalToStart();
    stopContinuous();
    delay(1000);
}

void loop()
{
    // sweepPositionalForward();
    runContinuousForward();
    stopContinuous();

    // sweepPositionalBackward();
    runContinuousReverse();
    stopContinuous();
}
