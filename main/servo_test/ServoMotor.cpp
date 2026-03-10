#include "Arduino.h"
#include <stdint.h>
#include "ServoMotor.h"

ServoMotor::ServoMotor() : pin(SERVO_1)
{
    servo.attach(pin);
    servo.write(0);
}

ServoMotor::ServoMotor(uint8_t servo_pin)
{
    pin = servo_pin;
    servo.attach(servo_pin);
    servo.write(90);
}

void ServoMotor::attach()
{
    servo.attach(pin);
}

void ServoMotor::detach()
{
    servo.detach();
}

void ServoMotor::setAngle(float angle, bool servo_type)
{
    short max_angle = servo_type ? SERVO_360_MAX : SERVO_180_MAX;
    angle = constrain(angle, 0, max_angle);
    servo.write(angle);
}

void ServoMotor::writeMicroseconds(int pulseWidth)
{
    servo.writeMicroseconds(pulseWidth);
}

void ServoMotor::setPeriodHertz(int frequency)
{
    servo.setPeriodHertz(frequency);
}

void ServoMotor::controlAngle(uint8_t control, bool servo_type)
{
    currentAngle = servo.read();
    currentAngle = control == 1 ? currentAngle + 10 : (control == -1 ? currentAngle - 10 : currentAngle);
    if (servo_type)
        currentAngle = constrain(currentAngle, 0, SERVO_360_MAX);
    else
        currentAngle = constrain(currentAngle, 0, SERVO_180_MAX);
    servo.write(currentAngle);
}

float ServoMotor::readAngle()
{
    return servo.read();
}

void ServoMotor::servoStop()
{
    servo.write(90);
}