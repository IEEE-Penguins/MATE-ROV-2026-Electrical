#include <Arduino.h>
#include "t100.h"
#include "GripperController.h"

ESCController esc;
GripperController gripper;

// Input structure from serial
typedef struct {
  int ROTATE_LEFT;
  int ROTATE_RIGHT;
  int FORWARD;
  int BACKWARD;
  int LEFT;
  int RIGHT;
  int UP;
  int DOWN;
  int Dpad_up;
  int Dpad_down;
  int Dpad_left;
  int Dpad_right;
} VALUES;

VALUES data;

// Parse input string
VALUES parseString(String input) {
  VALUES value;
  int values[12];
  char buffer[100];
  input.toCharArray(buffer, sizeof(buffer));
  char* token = strtok(buffer, ",");
  int i = 0;
  while (token && i < 12) {
    values[i++] = atoi(token);
    token = strtok(NULL, ",");
  }
  value.ROTATE_LEFT = values[0];
  value.ROTATE_RIGHT = values[1];
  value.FORWARD = values[2];
  value.BACKWARD = values[3];
  value.LEFT = values[4];
  value.RIGHT = values[5];
  value.UP = values[6];
  value.DOWN = values[7];
  value.Dpad_up = values[8];
  value.Dpad_down = values[9];
  value.Dpad_left = values[10];
  value.Dpad_right = values[11];
  return value;
}

void setup() {
  Serial.begin(115200);
  esc.init();
  gripper.init();
}

void loop() {
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    data = parseString(msg);
  }

  // Vertical movement: motors 0,1
  if (data.UP > 0) {
    esc.set(0, data.UP);
    esc.set(1, data.UP);
  } else if (data.DOWN > 0) {
    esc.set(0, data.DOWN, false);
    esc.set(1, data.DOWN, false);
  } else {
    esc.set(0, 0);
    esc.set(1, 0);
  }

  // Horizontal movement: motors 2–5
  if (data.FORWARD > 0) {
    esc.set(2, data.FORWARD);
    esc.set(3, data.FORWARD);
    esc.set(4, data.FORWARD, false);
    esc.set(5, data.FORWARD, false);
  } else if (data.BACKWARD > 0) {
    esc.set(2, data.BACKWARD, false);
    esc.set(3, data.BACKWARD, false);
    esc.set(4, data.BACKWARD);
    esc.set(5, data.BACKWARD);
  } else if (data.RIGHT > 0) {
    esc.set(2, data.RIGHT, false);
    esc.set(3, data.RIGHT);
    esc.set(4, data.RIGHT, false);
    esc.set(5, data.RIGHT);
  } else if (data.LEFT > 0) {
    esc.set(2, data.LEFT);
    esc.set(3, data.LEFT, false);
    esc.set(4, data.LEFT);
    esc.set(5, data.LEFT, false);
  } else if (data.ROTATE_RIGHT > 0) {
    esc.set(2, data.ROTATE_RIGHT);
    esc.set(3, data.ROTATE_RIGHT, false);
    esc.set(4, data.ROTATE_RIGHT, false);
    esc.set(5, data.ROTATE_RIGHT);
  } else if (data.ROTATE_LEFT > 0) {
    esc.set(2, data.ROTATE_LEFT, false);
    esc.set(3, data.ROTATE_LEFT);
    esc.set(4, data.ROTATE_LEFT);
    esc.set(5, data.ROTATE_LEFT, false);
  } else {
    for (int i = 2; i <= 5; i++) esc.set(i, 0);
  }

  // Gripper control (two independent servos)
  if (data.Dpad_right > 0) gripper.openRight();
  else if (data.Dpad_left > 0) gripper.closeRight();

  if (data.Dpad_up > 0) gripper.openLeft();
  else if (data.Dpad_down > 0) gripper.closeLeft();

  delay(20); // smooth signal
}
