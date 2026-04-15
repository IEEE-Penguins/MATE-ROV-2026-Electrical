#include "Depth.h"

Depth Sensor;

void setup() {
  Serial.begin(115200);

  Sensor.begin();

  Serial.println("Calibrating...");
  delay(3000);

  Sensor.calibrate(10);

  Serial.println("Done!");
}

void loop() {
  Serial.print("Pressure: ");
  Serial.print(Sensor.getPressure());
  Serial.print(" pa | ");

  Serial.print(Sensor.getDepthCM());
  Serial.println(" cm");
}