#ifndef SERVOMOTOR_H
#define SERVOMOTOR_H

#include <Arduino.h>
#include <ESP32Servo.h>

enum ServoType
{
    SERVO_180,
    SERVO_360
};

class ServoMotor
{
private:
    Servo myServo;
    int pin;
    ServoType type;

public:
    ServoMotor(int servoPin, ServoType servoType);
    void begin();

    // For 180° servo
    void setAngle(int angle);

    // For 360° servo
    void setSpeed(int speed);  
    void stop();
};

#endif