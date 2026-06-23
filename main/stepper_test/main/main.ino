#include <Arduino.h>
#include "GripperSubsystem.h" 

void setup() {
    Serial.begin(115200);
    delay(2000); 


    Serial.println(" 1. Auto Mode: Send a number from 0 to 100 (e.g., 50)");
    Serial.println(" 2. Joystick: Send 'V' + speed (e.g., V1000, V-500, V0)");
    Serial.println("========================================\n");

    GripperSubsystem::init(); 
    Serial.println("[System] Gripper Initialized & Ready to receive commands!");
}

void loop() {
    GripperSubsystem::update(); 

    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        if (input.length() == 0) return;

        input.toUpperCase(); 

        if (input.startsWith("V")) {
            String speedStr = input.substring(1);
            int32_t speed = speedStr.toInt();
            
            GripperSubsystem::joystickControl(speed);
            Serial.printf("[Test] Joystick Command: Motor moving at %d Hz\n", speed);
        }
        else {
            if (isDigit(input[0])) { 
                int percent = input.toInt();
                if (percent >= 0 && percent <= 100) {
                    GripperSubsystem::setPercent(percent);
                    Serial.printf("[Test] Auto Mode: Moving Gripper to %d%%\n", percent);
                } else {
                    Serial.println("[Error] Invalid Input! Send a number between 0 and 100.");
                }
            } else {
                Serial.println("[Error] Unknown Command! Use 0-100 or V<speed>.");
            }
        }
    }
}