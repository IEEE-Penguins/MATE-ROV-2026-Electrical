#ifndef SERVO_H
#define SERVO_H

#include <Arduino.h>
#include <ESP32Servo.h>

namespace ServoConfig
{
    // =====================================================
    // Final servo pin map (servo module view)
    // =====================================================
    // Positional servos
    static constexpr uint8_t POSITIONAL_SERVO_1_PIN = 32;
    static constexpr uint8_t POSITIONAL_SERVO_2_PIN = 33;

    // Continuous rotation servos
    static constexpr uint8_t CONTINUOUS_SERVO_1_PIN = 16;
    static constexpr uint8_t CONTINUOUS_SERVO_2_PIN = 17;

    static constexpr uint8_t POSITIONAL_SERVO_COUNT = 2;
    static constexpr uint8_t CONTINUOUS_SERVO_COUNT = 2;
    static constexpr uint8_t TOTAL_SERVO_COUNT = 4;

    // -----------------------------------------------------
    // Backward-compatible aliases for older unit tests
    // -----------------------------------------------------
    static constexpr uint8_t POSITIONAL_SERVO_TEST_PIN = POSITIONAL_SERVO_1_PIN;
    static constexpr uint8_t CONTINUOUS_SERVO_TEST_PIN = POSITIONAL_SERVO_2_PIN;

    // =====================================================
    // Common PWM configuration
    // =====================================================
    static constexpr int SERVO_FREQUENCY_HZ = 50;

    // Typical servo pulse range
    static constexpr int DEFAULT_MIN_PULSE_US = 1000;
    static constexpr int DEFAULT_MAX_PULSE_US = 2000;

    // =====================================================
    // Positional servo defaults
    // =====================================================
    static constexpr float POSITIONAL_MIN_ANGLE_DEG = 0.0f;
    static constexpr float POSITIONAL_MAX_ANGLE_DEG = 180.0f;
    static constexpr float POSITIONAL_HOME_ANGLE_DEG = 90.0f;
    static constexpr float POSITIONAL_STEP_DEG = 10.0f;

    // =====================================================
    // Continuous servo defaults
    // =====================================================
    static constexpr int CONTINUOUS_MIN_PULSE_US = 1100;
    static constexpr int CONTINUOUS_NEUTRAL_PULSE_US = 1500;
    static constexpr int CONTINUOUS_MAX_PULSE_US = 1900;
}

// ======================================================
// Base class: shared low-level servo hardware handling
// ======================================================
class ServoBase
{
public:
    explicit ServoBase(
        uint8_t pin,
        int minPulseUs = ServoConfig::DEFAULT_MIN_PULSE_US,
        int maxPulseUs = ServoConfig::DEFAULT_MAX_PULSE_US
    );

    virtual ~ServoBase() = default;

    virtual void begin();
    void attach();
    void detach();

    bool isAttached() const;
    uint8_t getPin() const;

    void setFrequencyHz(int frequencyHz);
    int getFrequencyHz() const;
    int getLastPulseUs() const;

protected:
    void ensureAttached();
    void writePulseUs(int pulseUs);

protected:
    Servo servo;
    uint8_t pin;
    bool attached;
    int minPulseUs;
    int maxPulseUs;
    int frequencyHz;
    int lastPulseUs;
};

// ======================================================
// 180-degree positional servo
// ======================================================
class PositionalServo : public ServoBase
{
public:
    PositionalServo(
        uint8_t pin,
        float minAngleDeg = ServoConfig::POSITIONAL_MIN_ANGLE_DEG,
        float maxAngleDeg = ServoConfig::POSITIONAL_MAX_ANGLE_DEG,
        float homeAngleDeg = ServoConfig::POSITIONAL_HOME_ANGLE_DEG,
        int minPulseUs = ServoConfig::DEFAULT_MIN_PULSE_US,
        int maxPulseUs = ServoConfig::DEFAULT_MAX_PULSE_US
    );

    void begin() override;

    void setAngle(float angleDeg);
    void stepBy(float deltaDeg);
    void moveHome();

    float getAngle() const;
    float getMinAngle() const;
    float getMaxAngle() const;
    float getHomeAngle() const;

private:
    float minAngleDeg;
    float maxAngleDeg;
    float homeAngleDeg;
    float currentAngleDeg;
};

// ======================================================
// 360 continuous rotation servo
// ======================================================
class ContinuousServo : public ServoBase
{
public:
    ContinuousServo(
        uint8_t pin,
        int minPulseUs = ServoConfig::CONTINUOUS_MIN_PULSE_US,
        int neutralPulseUs = ServoConfig::CONTINUOUS_NEUTRAL_PULSE_US,
        int maxPulseUs = ServoConfig::CONTINUOUS_MAX_PULSE_US
    );

    void begin() override;

    void setSpeed(float speed);
    void stop();

    float getSpeed() const;
    int getNeutralPulseUs() const;
    int getMinPulseUs() const;
    int getMaxPulseUs() const;

private:
    int neutralPulseUs;
    float currentSpeed;
};

#endif
