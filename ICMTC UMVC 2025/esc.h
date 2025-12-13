#ifndef ESC_H
#define ESC_H

#include <config.h>
#include <Arduino.h>
#include <ESP32Servo.h>

class ESCChannel {

  private:
    const int freq = 50;
    const int mid_diff = (ESC_MAX_CC - ESC_MAX_CCW) / 2;
    int channel_pin;
    bool is_unidirectional;
    int set_signal;
    Servo driver;

  public:
    ESCChannel(
      int channel_pin,
      bool is_unidirectional = false,
      int set_signal = 1100
    ) {
      this->channel_pin = channel_pin;
      this->is_unidirectional = is_unidirectional;
      this->set_signal = set_signal;
    };
    void begin();
    void drive(float speed);
    
};

#endif  