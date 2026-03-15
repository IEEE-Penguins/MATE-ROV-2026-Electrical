#ifndef THRUSTER_H
#define THRUSTER_H

#include <Arduino.h>
#include <ESP32Servo.h>

/*
    ============================================================
    Module configuration
    ============================================================
*/

// Thruster pins
static const uint8_t THRUSTER_1_PIN = 4;
static const uint8_t THRUSTER_2_PIN = 16;
static const uint8_t THRUSTER_3_PIN = 20;
static const uint8_t THRUSTER_4_PIN = 17;
static const uint8_t THRUSTER_5_PIN = 5;
static const uint8_t THRUSTER_6_PIN = 23;

static const uint8_t THRUSTER_COUNT = 6;

// ESC pulse range in microseconds
static const int16_t ESC_US_MIN     = 1100;
static const int16_t ESC_US_NEUTRAL = 1500;
static const int16_t ESC_US_MAX     = 1900;

// ESC PWM frequency
static const uint8_t ESC_PWM_HZ = 50;

// Default arming delay
static const uint16_t ESC_ARM_DELAY_MS = 3000;

// Normalized deadband around zero command
static const float THRUSTER_DEFAULT_DEADBAND = 0.05f;

/*
    ============================================================
    Low-level ESC output channel
    ============================================================
*/
class ESCChannel
{
public:
    ESCChannel(
        uint8_t pin = THRUSTER_1_PIN,
        int16_t minUs = ESC_US_MIN,
        int16_t neutralUs = ESC_US_NEUTRAL,
        int16_t maxUs = ESC_US_MAX,
        uint8_t pwmHz = ESC_PWM_HZ
    );

    bool begin();
    void arm(uint16_t armDelayMs = ESC_ARM_DELAY_MS);
    void writeMicroseconds(int16_t pulseUs);
    void stop();

    bool isBegun() const;
    bool isArmed() const;

    uint8_t pin() const;
    int16_t minPulse() const;
    int16_t neutralPulse() const;
    int16_t maxPulse() const;
    int16_t lastPulse() const;

private:
    int16_t clampPulse(int16_t pulseUs) const;

private:
    uint8_t pin_;
    int16_t minUs_;
    int16_t neutralUs_;
    int16_t maxUs_;
    uint8_t pwmHz_;

    int16_t lastPulse_;
    bool begun_;
    bool armed_;

    Servo driver_;
};

/*
    ============================================================
    Single thruster configuration
    ============================================================
*/
struct ThrusterConfig
{
    uint8_t pin;
    int16_t minUs;
    int16_t neutralUs;
    int16_t maxUs;
    uint8_t pwmHz;
    float deadband;
    bool inverted;

    ThrusterConfig(
        uint8_t thrusterPin = THRUSTER_1_PIN,
        int16_t minPulse = ESC_US_MIN,
        int16_t neutralPulse = ESC_US_NEUTRAL,
        int16_t maxPulse = ESC_US_MAX,
        uint8_t pwmFrequency = ESC_PWM_HZ,
        float normalizedDeadband = THRUSTER_DEFAULT_DEADBAND,
        bool isInverted = false
    );
};

/*
    ============================================================
    Single physical thruster
    ============================================================
*/
class Thruster
{
public:
    explicit Thruster(const ThrusterConfig& config = ThrusterConfig());

    bool begin();
    void arm(uint16_t armDelayMs = ESC_ARM_DELAY_MS);

    void setNormalized(float command);
    void setMicroseconds(int16_t pulseUs);
    void stop();

    float lastNormalized() const;
    int16_t lastPulse() const;

    bool isBegun() const;
    bool isArmed() const;
    uint8_t pin() const;

private:
    static float clampNormalized(float x);
    int16_t mapNormalizedToPulse(float command) const;

private:
    ThrusterConfig config_;
    ESCChannel esc_;
    float lastNormalized_;
};

/*
    ============================================================
    Thruster bank / array controller
    ============================================================
*/
class Thrusters
{
public:
    Thrusters(Thruster* thrusters[], uint8_t count);

    void beginAll();
    void armAll(uint16_t armDelayMs = ESC_ARM_DELAY_MS);
    void stopAll();

    void setAll(const float commands[], uint8_t count);
    void set(uint8_t index, float command);
    void stop(uint8_t index);

    Thruster* get(uint8_t index);
    const Thruster* get(uint8_t index) const;

    uint8_t count() const;

private:
    bool validIndex(uint8_t index) const;

private:
    Thruster* thrusters_[THRUSTER_COUNT];
    uint8_t count_;
};

#endif