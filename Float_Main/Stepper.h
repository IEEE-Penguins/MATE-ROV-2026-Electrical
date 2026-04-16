#ifndef STEPPER_H
#define STEPPER_H

#include <Arduino.h>

class Stepper {
  private:
    float stepsPerRevolution = 200.0f;
    float stepAngle = 1.8f;
    float stepDelay = 1000.0f;
    int currentPosition = 0;
    short dirPin;
    short stepPin;

  public:
    void begin(short dir_pin = 5, short step_pin = 18);
    void setSpeed(float stepsPerSecond);
    void editStepsPerRevolution(float steps);
    void moveToAngle(float angle);
    void moveSteps(int steps);
    void moveSingleDirection(bool dir);
    int getCurrentPosition();
};

#endif
