#include <depth.h>


void DepthSensor::begin() {
  depthSerial.begin(115200, SERIAL_8N1, 25, 13);
  Serial.begin(115200);
}

float DepthSensor::read() {
  int value = 0;
  
  if(depthSerial.available()) {
    value = depthSerial.parseInt();
  }
delay(5);
  //> insert mapping logic here

  return (float) value;
} 