#include "ServoMotor.h"

ServoMotor::ServoMotor(int servoPin, ServoType servoType)
{
    pin = servoPin;
    type = servoType;
}

void ServoMotor::begin()
{
    myServo.setPeriodHertz(50);
    myServo.attach(pin, 500, 2500);
}

void ServoMotor::setAngle(int angle)
{
    if (type == SERVO_180)
    {
        angle = constrain(angle, 0, 180);
        myServo.write(angle);
    }
}

void ServoMotor::setSpeed(int speed)
{
    if (type == SERVO_360)
    {
        speed = constrain(speed, -100, 100);

        int pwmValue = map(speed, -100, 100, 0, 180);
        myServo.write(pwmValue);
    }
}

void ServoMotor::stop()
{
    if (type == SERVO_360)
    {
        myServo.write(90);  // Stop position
    }
}