#ifndef PCA_H
#define PCA_H

#include "config.h"
#include <stdlib.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

struct Motor
{
    byte type;
    byte channel;
};

class PCA
{
private:
    Adafruit_PWMServoDriver pwmDriver = Adafruit_PWMServoDriver();
    byte freq = 50;
    byte Motor_count = ESCs_CHANNELS_NUM + SERVOS_CHANNELS_NUM;
    Motor Motors[ESCs_CHANNELS_NUM + SERVOS_CHANNELS_NUM];

public:
    PCA();
    void setFrequency(float freq);
    void pwmDrive(byte Motor_type, short value, byte channel);
    void resetClosedLoop(Motor Motor);
    void resetOpenLoop(Motor Motor);
    void closedLoopDrive(Motor Motor, char dir);
    void openLoopDrive(Motor Motor, char dir);
    void beginPCA(Motor motors[], byte new_count);

};

#endif // PCA_H