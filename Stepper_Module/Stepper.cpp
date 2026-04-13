#include "Stepper.h"

void Stepper::begin(short dir_pin, short step_pin) {
    dirPin = dir_pin;
    stepPin = step_pin;
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
}

void Stepper::setSpeed(float stepsPerSecond) {
    stepDelay = 1000000.0 / (2 * stepsPerSecond);
}

void Stepper::editStepsPerRevolution(float steps) {
    stepsPerRevolution = steps;
    stepAngle = 360.0 / stepsPerRevolution;
}

void Stepper::moveToAngle(float angle) {
    int targetSteps = angle / stepAngle;
    int stepsToMove = targetSteps - currentPosition;
    if (stepsToMove > 0) {
        digitalWrite(dirPin, HIGH);
    } else {
        digitalWrite(dirPin, LOW);
        stepsToMove = -stepsToMove;
    }
    for (int i = 0; i < stepsToMove; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(stepDelay);
    }
    currentPosition = targetSteps;
}

void Stepper::moveSingleDirection(bool dir) {
    digitalWrite(dirPin, dir);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
    currentPosition += (dir ? 1 : -1);
}

int Stepper::getCurrentPosition() {
    return currentPosition;
}

void Stepper::moveSteps(int steps) {
    if (steps > 0) {
        digitalWrite(dirPin, HIGH);
    } else {
        digitalWrite(dirPin, LOW);
        steps = -steps;
    }
    for (int i = 0; i < steps; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(stepDelay);
    }
    currentPosition += (digitalRead(dirPin) == HIGH) ? steps : -steps;
}