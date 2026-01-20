#include "PCA.h"

PCA::PCA()
{
}

void PCA::setFrequency(float freq)
{
    this->freq = freq;
    pwmDriver.setPWMFreq(freq);
}

void PCA::pwmDrive(byte Motor_type, short value, byte channel)
{
    if (Motor_type == SERVO_TYPE)
    {
        short pwmValue = map(value, 0, 180, SERVO_MIN, SERVO_MAX);
        pwmDriver.setPWM(channel, 0, pwmValue);
    }
    else if (Motor_type == THRUSTER_TYPE)
    {
        pwmDriver.writeMicroseconds(channel, value);
    }
}

void PCA::resetClosedLoop(Motor Motor)
{
    if (Motor.type == THRUSTER_TYPE)
        pwmDriver.writeMicroseconds(Motor.channel, ESC_STOP);
    else if (Motor.type == SERVO_TYPE)
        pwmDriver.setPWM(Motor.channel, 0, SERVO_MIN);
}

void PCA::resetOpenLoop(Motor Motor)
{
    if (Motor.type == THRUSTER_TYPE)
        pwmDriver.writeMicroseconds(Motor.channel, ESC_STOP);
    else if (Motor.type == SERVO_TYPE)
        pwmDriver.setPWM(Motor.channel, 0, 350);
}

void PCA::closedLoopDrive(Motor Motor, char dir)
{
    if (dir == 0)
        return;
    short current_angle = pwmDriver.getPWM(Motor.channel, true);
    short target_angle = constrain(current_angle += dir * SERVO_STEP, SERVO_MIN, SERVO_MAX);
    pwmDriver.setPWM(Motor.channel, 0, target_angle);
}

void PCA::openLoopDrive(Motor Motor, char dir)
{
    if (dir == 1 && Motor.type == SERVO_TYPE)
        pwmDriver.setPWM(Motor.channel, 0, 250);
    else if (dir == -1 && Motor.type == SERVO_TYPE)
        pwmDriver.setPWM(Motor.channel, 0, 450);
    else
        pwmDriver.setPWM(Motor.channel, 0, 350);
}

void PCA::beginPCA(Motor motors[], byte new_count)
{
    pwmDriver.begin();
    pwmDriver.setPWMFreq(freq);
    pwmDriver.setOscillatorFrequency(29666666);
    Motor_count = new_count;
    for (byte i = 0; i < Motor_count; i++)
    {
        if (motors[i].type == THRUSTER_TYPE)
            pwmDriver.writeMicroseconds(i, 1500);
        else if (motors[i].type == SERVO_TYPE)
            pwmDriver.setPWM(motors[i].channel, 0, SERVO_MIN);
    }

    delay(3000); // Allow ESCs to initialize
}