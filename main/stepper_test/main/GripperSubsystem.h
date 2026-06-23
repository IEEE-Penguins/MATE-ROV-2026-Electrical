#ifndef GRIPPER_SUBSYSTEM_H
#define GRIPPER_SUBSYSTEM_H

#include <Arduino.h>

/**
 * @namespace GripperSubsystem
 * @brief Facade pattern subsystem to encapsulate all Gripper operations.
 * Isolates the main application logic from low-level motor drivers and configurations.
 */
namespace GripperSubsystem {

    // --- Core Lifecycle ---
    void init();
    void update();
    
    // --- Discrete Control Commands ---
    void open();
    void close();
    void setPercent(uint8_t percent);

    // --- Continuous/Analog Control ---
    void joystickControl(int32_t speed);

}

#endif // GRIPPER_SUBSYSTEM_H