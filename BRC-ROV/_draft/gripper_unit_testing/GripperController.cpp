#include "GripperController.h"

void GripperController::init() {
  leftServo.attach(leftPin);
  rightServo.attach(rightPin);
  closeLeft();
  closeRight();
}

void GripperController::openRight() {
  rightServo.write(openAngle);
}

void GripperController::closeRight() {
  rightServo.write(closeAngle);
}

void GripperController::openLeft() {
  leftServo.write(openAngle);
}

void GripperController::closeLeft() {
  leftServo.write(closeAngle);
}

