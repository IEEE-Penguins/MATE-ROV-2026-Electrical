#ifndef DEPTH_H
#define DEPTH_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

class DepthSensor {
public:
    enum Status : uint8_t {
        OK = 0,
        NOT_READY,
        DISCONNECTED,
        OUT_OF_RANGE_LOW,
        OUT_OF_RANGE_HIGH
    };

    // Default calibration values are the voltage-domain equivalents
    // of the old current-based calibration:
    // I_ZERO = 3.88 mA  -> V_ZERO = 0.8536 V  (with 220 ohm shunt)
    // I_CAL  = 4.17 mA  -> V_CAL  = 0.9174 V  (with 220 ohm shunt)
    // DEPTH_CAL = 0.203 m
    DepthSensor(TwoWire &w);
    DepthSensor(TwoWire &w,
                float zeroVoltage,
                float calVoltage,
                float calDepthMeters,
                float maxDepthMeters = 5.0f,
                uint8_t channel = 0,
                uint8_t i2cAddress = 0x48);

    bool begin();
    bool update();

    // Calibration / configuration
    void setCalibration(float zeroVoltage, float calVoltage, float calDepthMeters);
    void setMaxDepth(float maxDepthMeters);

    // Data access
    int16_t rawAdc() const;
    float voltage() const;
    float depthMeters() const;
    float depthCentimeters() const;

    // State
    Status status() const;
    bool isReady() const;

    // Helpers
    static const char* statusToString(Status s);

private:
    TwoWire* wire_;
    Adafruit_ADS1115 ads_;

    uint8_t i2cAddress_;
    uint8_t channel_;

    // Fixed ADC->voltage conversion for current setup
    static constexpr float ADC_TO_VOLT = 0.000125f;

    // Calibration
    float zeroVoltage_;
    float calVoltage_;
    float calDepthMeters_;
    float voltsPerMeter_;
    float maxDepthMeters_;

    // Cached sample
    int16_t rawAdc_;
    float voltage_;
    float depthMeters_;
    Status status_;
    bool ready_;

    void updateScale();
    float calcDepthFromVoltage(float voltage) const;
    Status evaluateStatus(float voltage) const;
};

#endif