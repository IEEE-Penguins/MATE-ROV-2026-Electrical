#ifndef ROV_CONFIG_H
#define ROV_CONFIG_H

#include <Arduino.h>

/**
 * @namespace RovPins
 * @brief Hardware pin definitions for the ROV ESP32 controller.
 */
namespace RovPins {
    static constexpr uint8_t GRIPPER_STEP = 33;
    static constexpr uint8_t GRIPPER_DIR = 25;
}

/**
 * @namespace GripperSpecs
 * @brief Kinematic specifications and software limits for the Gripper mechanism.
 */
namespace GripperSpecs {
    // --- Motion Profile ---
    static constexpr uint32_t MAX_SPEED_HZ = 2000;  // Maximum allowed stepping frequency
    static constexpr uint32_t ACCELERATION = 500;   // Acceleration rate (steps/sec^2) for smooth starts/stops

    // --- Software Endstops ---
    static constexpr int32_t MIN_POSITION = 0;      // Physical limit: Fully Closed
    static constexpr int32_t MAX_POSITION = 5000;   // Physical limit: Fully Opened
}

#endif // ROV_CONFIG_H