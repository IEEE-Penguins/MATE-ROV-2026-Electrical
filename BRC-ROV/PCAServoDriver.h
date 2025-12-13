#ifndef PCASERVODRIVER_H
#define PCASERVODRIVER_H

#include <config.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

class ServosController {
  public:
    void begin();
    void reset_closed_loop(int channel);
    void reset_open_loop(int channel);
    void open_loop_drive(int channel, int dir);
    void closed_loop_drive(int channel, int dir);
    
  private:
    const int freq = 50;
    Adafruit_PWMServoDriver driver = Adafruit_PWMServoDriver();
};

#endif  