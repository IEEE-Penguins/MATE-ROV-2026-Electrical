#ifndef DEPTH_H
#define DEPTH_H

#include <Arduino.h>

class Depth {
  private:
    uint8_t doutPin;
    uint8_t sckPin;
    long offset = 0;
    float calibrationFactor = 0.007f;
    uint8_t numSamples = 10;
    long val = 0;

    bool waitReady(uint32_t timeoutUs);
    long readRaw();

  public:
    void begin(uint8_t dout_pin = 14, uint8_t sck_pin = 21);
    void calibrate(uint8_t samples = 10);
    void setCalibrationFactor(float factor);
    void setSamples(uint8_t samples);
    long read();
    float getPressure();
    float getDepthMeters();
    float getDepthCM();
};

#endif
