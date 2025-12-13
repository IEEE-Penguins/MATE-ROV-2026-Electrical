#include <depth.h>

DepthSensor depthSensor;

void setup() {
  depthSensor.begin();
  Serial.begin(115200);
}

void loop() {

  delay(50);

  Serial.print("depth: "); Serial.println(depthSensor.read());

  delay(50);
  
}