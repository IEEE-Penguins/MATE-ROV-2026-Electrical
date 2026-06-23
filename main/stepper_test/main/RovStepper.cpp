#include "RovStepper.h"

FastAccelStepperEngine RovStepper::engine = FastAccelStepperEngine();

RovStepper::RovStepper() :
      stepper(NULL), 
      minPosition(0),               
      maxPosition(0),
      wasRunning(false),            
      isInitialized(false),
      onTargetReached(nullptr),
      lastStopTime(0),
      needsSaving(false)
{
}

void RovStepper::initEngine() {
    engine.init();
}

bool RovStepper::begin(uint8_t stepPin, uint8_t dirPin, const char* memoryName) {
    this->nvsName = memoryName;
    stepper = engine.stepperConnectToPin(stepPin);

    if (stepper) {
        stepper->setDirectionPin(dirPin);
        
        stepper->setSpeedInHz(StepperConfig::DEFAULT_MAX_SPEED_HZ);
        stepper->setAcceleration(StepperConfig::DEFAULT_ACCELERATION);
        
        // --- Flash Memory Recovery Sequence ---
        preferences.begin(nvsName.c_str(), false);
        int32_t savedPos = preferences.getInt("pos", 0); // Default to 0 if no record exists
        stepper->setCurrentPosition(savedPos);

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

void RovStepper::emergencyStop() {
    if (isInitialized && stepper) {
        stepper->forceStop(); // Halts immediately without losing absolute position track
    }
}

void RovStepper::stop() {
    if (isInitialized && stepper) {
        stepper->stopMove(); // Decelerates gracefully
    }
}

bool RovStepper::isRunning() const {
    return (isInitialized && stepper) ? stepper->isRunning() : false;
}

int32_t RovStepper::getCurrentPosition() const {
    return (isInitialized && stepper) ? stepper->getCurrentPosition() : 0;
}

void RovStepper::setSoftwareLimits(int32_t minPos, int32_t maxPos) {
    this->minPosition = minPos;
    this->maxPosition = maxPos;
}

void RovStepper::moveGripperToPercent(uint8_t percent) {
    if (!isInitialized || !stepper) return;
    
    // Clamp input to valid percentage range
    if (percent > 100) percent = 100;
    
    // Map percentage to physical steps within software endstops
    int32_t targetStep = map(percent, 0, 100, minPosition, maxPosition);
    
    wasRunning = true; // Flag for edge-detection in update()
    stepper->moveTo(targetStep); 
}

void RovStepper::joystickMove(int32_t speed) {
    if (!isInitialized || !stepper) return;

    // Dynamically adjust speed while strictly adhering to positional limits
    if (speed > 0) { 
        stepper->setSpeedInHz(speed);
        stepper->moveTo(maxPosition); 
        wasRunning = true;
    } 
    else if (speed < 0) { 
        stepper->setSpeedInHz(abs(speed));
        stepper->moveTo(minPosition);
        wasRunning = true;
    } 
    else {
        stepper->stopMove();
    }
}
void RovStepper::setOnTargetReached(ActionCallback callback) {
    onTargetReached = callback;
}

void RovStepper::update() {
    if (!isInitialized || !stepper) return;

    bool currentlyRunning = stepper->isRunning();

    // Edge Detection: Motor just stopped moving
    if (wasRunning && !currentlyRunning) {
        wasRunning = false; 
        needsSaving = true;        
        lastStopTime = millis(); // Start NVS wear-leveling timer
        
        // Trigger asynchronous callback
        if (onTargetReached) {
            onTargetReached(stepper->getCurrentPosition());
        }
    }

    // NVS Wear-Leveling Protection: Delay flash write by 2000ms 
    // to prevent memory degradation during rapid joystick oscillations
    if (needsSaving && !currentlyRunning) {
        if (millis() - lastStopTime > 2000) { 
            savePositionToFlash();
            needsSaving = false; 
        }
    }
}

void RovStepper::savePositionToFlash() {
    if (isInitialized) {
        preferences.putInt("pos", stepper->getCurrentPosition());
    }
}

void RovStepper::powerOn() {
    if (isInitialized && stepper) {
        stepper->enableOutputs(); 
    }
}

void RovStepper::powerOff() {
    if (isInitialized && stepper) {
        stepper->disableOutputs(); 
    }
}