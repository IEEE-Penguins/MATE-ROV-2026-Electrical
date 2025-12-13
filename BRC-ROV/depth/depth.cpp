#include <depth.h>

void DepthSensor::begin() {
  pinMode(DEPTH_SENSOR, INPUT);
}

float DepthSensor::read() {
  float value = 0.0;

  //[?]- add your mapping logic here 

  return value;
} 