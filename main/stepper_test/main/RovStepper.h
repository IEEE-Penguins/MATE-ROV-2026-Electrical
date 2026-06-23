#ifndef ROV_STEPPER_H
#define ROV_STEPPER_H

#include <Arduino.h>
#include "FastAccelStepper.h"
#include <Preferences.h>
#include <functional>

/**
 * @namespace StepperConfig
 * @brief Default fallback parameters for motor initialization.
 */
namespace StepperConfig {
    static constexpr uint32_t DEFAULT_MAX_SPEED_HZ = 1000;
    static constexpr uint32_t DEFAULT_ACCELERATION = 500;
}

/**
 * @class RovStepper
 * @brief A robust, NVS-backed stepper driver abstraction.
 * Features software endstops, flash memory recovery, and non-blocking callbacks.
 */
class RovStepper {
public:
    // Defines a callback signature for asynchronous target-reached events
    using ActionCallback = std::function<void(int32_t finalPosition)>;

    RovStepper();

    static void initEngine();

    // Initializes motor, binds pins, and restores last known position from Flash (NVS)
    bool begin(uint8_t stepPin, uint8_t dirPin, const char* memoryName = "gripper");

    void setMotionProfile(uint32_t maxSpeedHz, uint32_t acceleration);

    void stop();                             // Soft deceleration stop
    void emergencyStop();                    // Instantaneous stop (retains current position)

    bool isRunning() const;
    int32_t getCurrentPosition() const;
    
    // Safety bounds constraints
    void setSoftwareLimits(int32_t minPos, int32_t maxPos);
    
    // High-level Kinematic Commands
    void moveGripperToPercent(uint8_t percent); 
    void joystickMove(int32_t speed);
    
    // Event-driven callback registration
    void setOnTargetReached(ActionCallback callback);
    
    // Non-blocking background task (Must be called continuously in loop)
    void update();
    
    void powerOn();
    void powerOff();
private:
    static FastAccelStepperEngine engine; 
    FastAccelStepper* stepper;            
    
    int32_t minPosition;
    int32_t maxPosition;
    
    bool wasRunning;
    bool isInitialized;
    
    // NVS Memory Components
    Preferences preferences; 
    String nvsName;
    ActionCallback onTargetReached;
    
    // Wear-leveling logic variables for NVS protection
    unsigned long lastStopTime;
    bool needsSaving;
    
    void savePositionToFlash();
};

#endif // ROV_STEPPER_H