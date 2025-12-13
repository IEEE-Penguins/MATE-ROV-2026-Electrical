#ifndef THRUSTERS_H
#define THRUSTERS_H

#include <../config.h>
#include <Arduino.h>
#include <ESP32Servo.h>

class ThrustersController {
  public:
    void begin();
    void drive(float speed_array[ESCs_CHANNELS_NUM]);
    
  private:
    const int freq = 50;
    const int mid_diff = (ESC_MAX_CC - ESC_MAX_CCW) / 2;
    const int pin[5] = {ESC_1_2, ESC_3, ESC_4, ESC_5, ESC_6};
    Servo esc[ESCs_CHANNELS_NUM];
};

#endif  