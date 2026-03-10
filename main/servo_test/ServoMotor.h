#ifndef SERVOMOTOR_H
#define SERVOMOTOR_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"

class ServoMotor
{
public:
    ServoMotor();
    ServoMotor(uint8_t pin);
    void attach();
    void detach();
    void setAngle(float angle, bool servo_type);
    void writeMicroseconds(int pulseWidth);
    float readAngle();
    void setPeriodHertz(int frequency);
    void controlAngle(uint8_t control, bool servo_type);
    void servoStop();

private:
    Servo servo;
    uint8_t pin;
    uint8_t currentAngle = 0;
};

#endif