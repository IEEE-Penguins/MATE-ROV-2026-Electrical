#ifndef DEPTH_H
#define DEPTH_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

#define SDA_PIN 21
#define SCL_PIN 22

#define SHUNT_R 220.0
#define MAX_DEPTH 5.0

#define I_ZERO 3.88
#define I_CAL 4.17
#define DEPTH_CAL 0.203

class DepthSensor
{
private:
    Adafruit_ADS1115 ads;
    float mA_per_meter;

    float calcDepth(float current);
    void checkStatus(float current, float voltage);

public:
    DepthSensor();
    bool begin();
    void readSensor();
    void printCalibration();
};

#endif