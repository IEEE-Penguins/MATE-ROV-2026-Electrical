#include <Arduino.h>
#include "Stepper.h"

Stepper motor;

String inputString = "";
bool inputComplete = false;

void setup() {
    Serial.begin(115200);

    motor.begin(5, 18);   // DIR = 5, STEP = 18

    Serial.println("Enter speed in steps/sec:");
    
    // wait for speed input
    while (!Serial.available());
    float speed = Serial.parseFloat();

    motor.setSpeed(speed);

    Serial.print("Speed set to: ");
    Serial.println(speed);

    Serial.println("Now enter angles:");
}

void loop() {
    if (Serial.available()) {
        float angle = Serial.parseFloat();

        Serial.print("Moving to angle: ");
        Serial.println(angle);

        motor.moveToAngle(angle);

        Serial.print("Current position: ");
        Serial.println(motor.getCurrentPosition());

        Serial.println("Enter next angle:");
    }
}