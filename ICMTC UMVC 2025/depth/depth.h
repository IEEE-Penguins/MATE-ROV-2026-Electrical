#ifndef DEPTH_H
#define DEPTH_H

#include <../config.h>
#include <Arduino.h>

class DepthSensor {
  public:
    void begin();
    float read();
};

#endif  