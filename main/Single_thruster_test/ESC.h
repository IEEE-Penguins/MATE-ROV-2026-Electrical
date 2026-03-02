#ifndef ESC_H
#define ESC_H

#include "config.h"
#include <Arduino.h>
#include <ESP32Servo.h>

class ESCChannel
{
private:
    byte freq = 50;
    short stop_signal = 1500;
    byte channel_pin;
    short set_signal;
    short last_signal;
    Servo driver;

public:
    ESCChannel();
    ESCChannel(byte pin, short signal);
    void drive(short signal);
    void arm();
    void stop();
};

#endif