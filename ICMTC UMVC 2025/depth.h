#ifndef DEPTH_H
#define DEPTH_H

#include <config.h>
#include <Arduino.h>
#include <HardwareSerial.h>


class DepthSensor {
  public:
    void begin();
    float read();

  private:
    HardwareSerial depthSerial = HardwareSerial(2);
};

#endif  