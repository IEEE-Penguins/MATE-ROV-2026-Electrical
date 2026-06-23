#include "GripperSubsystem.h"
#include "RovStepper.h"
#include "RovConfig.h"

namespace GripperSubsystem {
    
    // Hidden private instance of the motor driver
    RovStepper motor;

    // Internal callback to handle telemetry when motor operations conclude
    void onTargetReached(int32_t pos) {
        Serial.printf("[Gripper System] Action Complete. Position saved: %d\n", pos);
    }

    void init() {
        RovStepper::initEngine();
        
        // Initialize hardware mapping
        motor.begin(RovPins::GRIPPER_STEP, RovPins::GRIPPER_DIR, "gripper");        
        // Inject configuration parameters
        motor.setMotionProfile(GripperSpecs::MAX_SPEED_HZ, GripperSpecs::ACCELERATION);
        motor.setSoftwareLimits(GripperSpecs::MIN_POSITION, GripperSpecs::MAX_POSITION);
        
        // Register event hooks
        motor.setOnTargetReached(onTargetReached);
    }

    void update() {
        // Polls the underlying driver for state changes (Non-blocking)
        motor.update();
    }

    void open() {
        motor.moveGripperToPercent(100);
    }

    void close() {
        motor.moveGripperToPercent(0);
    }

    void setPercent(uint8_t percent) {
        motor.moveGripperToPercent(percent);
    }

    void joystickControl(int32_t speed) {
        motor.joystickMove(speed);
    }


    void powerOn() {
        motor.powerOn();
    }

    void powerOff() {
        motor.powerOff();
    }
}