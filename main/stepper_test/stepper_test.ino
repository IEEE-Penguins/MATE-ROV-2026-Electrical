#include <Arduino.h>
#include "stepper.h"


namespace ROVPins {
    static constexpr uint8_t TEST_STEP_PIN = 33;
    static constexpr uint8_t TEST_DIR_PIN = 25;
}

RovStepper testStepper;

unsigned long lastPrintTime = 0;
const unsigned long PRINT_INTERVAL = 1000;

void setup() {
    Serial.begin(460800);
    Serial.println("\n--- ROV Stepper Test ---");

    RovStepper::initEngine();

    if (testStepper.begin(ROVPins::TEST_STEP_PIN, ROVPins::TEST_DIR_PIN)) {
        Serial.println("[OK] Stepper Initialized Successfully.");
    } else {
        Serial.println("[ERROR] Failed to init stepper! Check hardware timers.");
    }


    testStepper.setMotionProfile(2000, 500);
    Serial.println("[OK] Motion Profile Set (Max: 2000, Accel: 500)");

}

void loop() {

    if (Serial.available() > 0) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();         
        cmd.toUpperCase();  

        if (cmd.startsWith("V")) {

            int32_t targetSpeed = cmd.substring(1).toInt(); 
            testStepper.setContinuousSpeed(targetSpeed);
            Serial.printf(">>> Command: Speed set to %d Hz\n", targetSpeed);
        } 
        else if (cmd == "S") {
            testStepper.stop();
            Serial.println(" Command: SOFT STOP TRIGGERED");
        } 
        else if (cmd == "E") {
            testStepper.emergencyStop();
            Serial.println(" Command: EMERGENCY STOP TRIGGERED");
        }
        else {
            Serial.println(" Error: Unknown Command");
        }
    }


    unsigned long currentMillis = millis();
    if (currentMillis - lastPrintTime >= PRINT_INTERVAL) {
        lastPrintTime = currentMillis;

        bool isMoving = testStepper.isRunning();
        int32_t currentPos = testStepper.getCurrentPosition();

        Serial.printf("[Telemetry] Status: %s | Position (Steps): %d\n", 
                      isMoving ? "MOVING" : "IDLE  ", currentPos);
    }
}
/*
const int dirPin = 25;   
const int stepPin = 33;  

void setup() {
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  
  Serial.begin(115200);
  
}

void loop() {
  digitalWrite(dirPin, HIGH); 
  
  for(int x = 0; x < 200; x++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(2000);   
    digitalWrite(stepPin, LOW);
    delayMicroseconds(2000);   
  }
  
  delay(1000);

  digitalWrite(dirPin, LOW);
  
  for(int x = 0; x < 200; x++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(2000);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(2000);
  }
  
  delay(1000); 
}*/