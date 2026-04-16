#include "Stepper.h"

#include <math.h>

namespace {
uint32_t safeDelayMicros(float stepDelayUs) {
  if (stepDelayUs < 1.0f) {
    return 1U;
  }
  return (uint32_t)stepDelayUs;
}

void pulseStep(short stepPin, uint32_t delayUs) {
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(delayUs);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(delayUs);
}
}  // namespace

void Stepper::begin(short dir_pin, short step_pin) {
  dirPin = dir_pin;
  stepPin = step_pin;
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
}

void Stepper::setSpeed(float stepsPerSecond) {
  if (stepsPerSecond <= 0.0f) {
    stepsPerSecond = 1.0f;
  }
  stepDelay = 1000000.0f / (2.0f * stepsPerSecond);
  if (stepDelay < 1.0f) {
    stepDelay = 1.0f;
  }
}

void Stepper::editStepsPerRevolution(float steps) {
  if (steps <= 0.0f) {
    return;
  }
  stepsPerRevolution = steps;
  stepAngle = 360.0f / stepsPerRevolution;
}

void Stepper::moveToAngle(float angle) {
  if (stepAngle <= 0.0f) {
    return;
  }

  int targetSteps = (int)lroundf(angle / stepAngle);
  int stepsToMove = targetSteps - currentPosition;

  if (stepsToMove == 0) {
    return;
  }

  bool dir = (stepsToMove > 0);
  int totalSteps = dir ? stepsToMove : -stepsToMove;

  digitalWrite(dirPin, dir ? HIGH : LOW);

  uint32_t delayUs = safeDelayMicros(stepDelay);
  for (int i = 0; i < totalSteps; i++) {
    pulseStep(stepPin, delayUs);
  }

  currentPosition = targetSteps;
}

void Stepper::moveSingleDirection(bool dir) {
  digitalWrite(dirPin, dir ? HIGH : LOW);
  pulseStep(stepPin, safeDelayMicros(stepDelay));
  currentPosition += (dir ? 1 : -1);
}

int Stepper::getCurrentPosition() {
  return currentPosition;
}

void Stepper::moveSteps(int steps) {
  if (steps == 0) {
    return;
  }

  bool dir = (steps > 0);
  int totalSteps = dir ? steps : -steps;

  digitalWrite(dirPin, dir ? HIGH : LOW);

  uint32_t delayUs = safeDelayMicros(stepDelay);
  for (int i = 0; i < totalSteps; i++) {
    pulseStep(stepPin, delayUs);
  }

  currentPosition += (dir ? totalSteps : -totalSteps);
}
