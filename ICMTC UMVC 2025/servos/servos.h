#ifndef SERVOS_H
#define SERVOS_H

#include <../config.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

class ServosController {
  public:
    void begin();
    void drive(int channel, int dir);
    
  private:
    const int freq = 50;
    Adafruit_PWMServoDriver driver = Adafruit_PWMServoDriver();
};

#endif  