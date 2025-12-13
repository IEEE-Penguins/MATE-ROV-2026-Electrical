#ifndef GRIPPER_CONTROLLER_H
#define GRIPPER_CONTROLLER_H

#include <Arduino.h>
#include <ESP32Servo.h>

class GripperController {
  public:
    void init();
    void openRight();
    void closeRight();
    void openLeft();
    void closeLeft();

  private:
    Servo leftServo;
    Servo rightServo;

    const int leftPin = 16;
    const int rightPin = 17;

    const int openAngle = 180;
    const int closeAngle = 0;
};

#endif

