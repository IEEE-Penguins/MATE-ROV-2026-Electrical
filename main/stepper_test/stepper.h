#ifndef STEPPER_H
#define STEPPER_H

#include <Arduino.h>
#include "FastAccelStepper.h"
#include <Preferences.h>
#include <functional>

namespace StepperConfig
{
    // Fixed motion profile. Runtime commands do not scale speed.
    // Any positive command moves at this speed toward maxPosition.
    // Any negative command moves at this speed toward minPosition.
    static constexpr uint32_t FIXED_MAX_SPEED_HZ = 2000;
    static constexpr uint32_t ACCELERATION = 500;

    // Default software limits. Override with setSoftwareLimits() after begin().
    static constexpr int32_t DEFAULT_MIN_POSITION = 0;
    static constexpr int32_t DEFAULT_MAX_POSITION = 5000;

    // Small zero-zone for joystick/noisy analog input.
    static constexpr float COMMAND_DEADBAND = 0.05f;

    // Flash write delay after movement stops, to reduce NVS wear.
    static constexpr uint32_t POSITION_SAVE_DELAY_MS = 2000;
}

class RovStepper
{
public:
    using ActionCallback = std::function<void(int32_t finalPosition)>;

    RovStepper();

    // Safe to call more than once. begin() also calls it automatically.
    static void initEngine();

    // Initializes the motor and restores the last saved position from NVS.
    bool begin(uint8_t stepPin, uint8_t dirPin, const char* memoryName = "stepper");

    // Optional configuration.
    void setSoftwareLimits(int32_t minPos, int32_t maxPos);
    void setAcceleration(uint32_t acceleration);
    void setCurrentPosition(int32_t position, bool saveNow = true);
    void setOnTargetReached(ActionCallback callback);

    // Normalized continuous command:
    //   command > +deadband : move toward maxPosition at fixed max speed
    //   command < -deadband : move toward minPosition at fixed max speed
    //   otherwise           : stop
    void setNormalized(float command);

    // Backward-friendly alias for joystick style control.
    void joystickMove(float command);

    // Optional absolute-position helpers. These also use the fixed max speed.
    void moveToPercent(uint8_t percent);
    void open();
    void close();

    void stop();
    void emergencyStop();

    // Must be called frequently from loop() for stop detection and NVS saving.
    void update();

    void powerOn();
    void powerOff();

    bool isReady() const;
    bool isRunning() const;
    int32_t getCurrentPosition() const;
    int32_t getMinPosition() const;
    int32_t getMaxPosition() const;
    float getLastCommand() const;
    int8_t getLastDirection() const;

private:
    static FastAccelStepperEngine engine;
    static bool engineInitialized;

    FastAccelStepper* stepper;
    Preferences preferences;
    String nvsName;

    int32_t minPosition;
    int32_t maxPosition;
    uint32_t acceleration;

    bool isInitialized;
    bool preferencesOpen;
    bool wasRunning;
    bool needsSaving;
    unsigned long lastStopTime;

    float lastCommand;
    int8_t lastDirection;

    ActionCallback onTargetReached;

    static float clampNormalized(float command);
    int32_t clampPosition(int32_t position) const;
    void moveTowardLimit(int8_t direction);
    void markPositionDirty();
    void savePositionToFlash();
};

#endif // STEPPER_H
