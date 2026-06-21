#include "stepper.h"


FastAccelStepperEngine RovStepper::engine = FastAccelStepperEngine();


RovStepper::RovStepper() : stepper(NULL), stepsPerRevolution(StepperConfig::DEFAULT_STEPS_PER_REV), isInitialized(false) {
}


void RovStepper::initEngine() {
    engine.init();
}


bool RovStepper::begin(uint8_t stepPin, uint8_t dirPin, float stepsPerRev) {
    this->stepsPerRevolution = stepsPerRev;

    stepper = engine.stepperConnectToPin(stepPin);

    if (stepper) {
        stepper->setDirectionPin(dirPin);
        
        stepper->setSpeedInHz(StepperConfig::DEFAULT_MAX_SPEED_HZ);
        stepper->setAcceleration(StepperConfig::DEFAULT_ACCELERATION);
        
        // stepper->setEnablePin(ENABLE_PIN);
        // stepper->setAutoEnable(true); 

        isInitialized = true;
        return true;
    }
    
    isInitialized = false;
    return false; 
}


void RovStepper::setMotionProfile(uint32_t maxSpeedHz, uint32_t acceleration) {
    if (isInitialized && stepper) {
        stepper->setSpeedInHz(maxSpeedHz);
        stepper->setAcceleration(acceleration);
    }
}


void RovStepper::setContinuousSpeed(int32_t speedHz) {
    if (!isInitialized || !stepper) return;

    if (speedHz == 0) {
        stop(); 
        return;
    }

    stepper->setSpeedInHz(abs(speedHz));

    if (speedHz > 0) {
        stepper->runForward();
    } else {
        stepper->runBackward();
    }
}


void RovStepper::stop() {
    if (isInitialized && stepper) {
        stepper->stopMove(); 
    }
}


void RovStepper::emergencyStop() {
    if (isInitialized && stepper) {
        stepper->forceStopAndNewPosition(0); 
    }
}

bool RovStepper::isRunning() const {
    if (isInitialized && stepper) {
        return stepper->isRunning();
    }
    return false;
}

int32_t RovStepper::getCurrentPosition() const {
    if (isInitialized && stepper) {
        return stepper->getCurrentPosition();
    }
    return 0;
}