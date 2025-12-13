#include <Arduino.h>
#include "GripperController.h"

GripperController gripper;

void setup() {
  Serial.begin(115200);
  gripper.init();

  Serial.println("Gripper Unit Testing Started");
}

void loop() {
  Serial.println("Opening Right Gripper...");
  gripper.openRight();
  delay(2000);

  Serial.println("Closing Right Gripper...");
  gripper.closeRight();
  delay(2000);

  Serial.println("Opening Left Gripper...");
  gripper.openLeft();
  delay(2000);

  Serial.println("Closing Left Gripper...");
  gripper.closeLeft();
  delay(2000);
}

