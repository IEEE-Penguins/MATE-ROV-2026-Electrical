#include "Thrusters.h"

Thrusters thrusterController;

float speeds[7][6] {
  {-.5, 0, 0, 0, 0, 0},
  {0, -.5, 0, 0, 0, 0},
  {0, 0, -.5, 0, 0, 0},
  {0, 0, 0, -.5, 0, 0},
  {0, 0, 0, 0, -.5, 0},
  {0, 0, 0, 0, 0, -.5},
  {-.5, -.5, -.5, -.5, -.5}
};

void setup() {
  Serial.begin(115200);
}

void loop() {
  for (byte i = 0; i < 6; i++) {
    Serial.print("Motor "); Serial.print(i + 1); Serial.println(" is running");
    thrusterController.drive(speeds[i]);
    delay(3000);
  }
}