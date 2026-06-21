#ifndef ROV_STEPPER_H
#define ROV_STEPPER_H

#include <Arduino.h>
#include "FastAccelStepper.h"

namespace StepperConfig {
    static constexpr float DEFAULT_STEPS_PER_REV = 200.0f;
    static constexpr uint32_t DEFAULT_MAX_SPEED_HZ = 1000;
    static constexpr uint32_t DEFAULT_ACCELERATION = 500;
  
}

class RovStepper {
public:

    RovStepper();

    static void initEngine();

    bool begin(uint8_t stepPin, uint8_t dirPin, float stepsPerRev = StepperConfig::DEFAULT_STEPS_PER_REV);

    void setMotionProfile(uint32_t maxSpeedHz, uint32_t acceleration);

    void setContinuousSpeed(int32_t speedHz);
    void stop();                             
    void emergencyStop();                  

   
    bool isRunning() const;
    int32_t getCurrentPosition() const;

private:
    static FastAccelStepperEngine engine; 
    
    FastAccelStepper* stepper;            
    
    float stepsPerRevolution;
    bool isInitialized;
};

#endif 