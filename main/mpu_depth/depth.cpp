#include "depth.h"

DepthSensor::DepthSensor(TwoWire &w)
    : wire_(&w),
      i2cAddress_(0x48),
      channel_(0),
      zeroVoltage_(0.8703f),
      calVoltage_(0.9068f),
      calDepthMeters_(0.11f),
      voltsPerMeter_(0.0f),
      maxDepthMeters_(5.0f),
      rawAdc_(0),
      voltage_(0.0f),
      depthMeters_(0.0f),
      status_(NOT_READY),
      ready_(false) {
    updateScale();
}

DepthSensor::DepthSensor(TwoWire &w,
                         float zeroVoltage,
                         float calVoltage,
                         float calDepthMeters,
                         float maxDepthMeters,
                         uint8_t channel,
                         uint8_t i2cAddress)
    : wire_(&w),
      i2cAddress_(i2cAddress),
      channel_(channel),
      zeroVoltage_(zeroVoltage),
      calVoltage_(calVoltage),
      calDepthMeters_(calDepthMeters),
      voltsPerMeter_(0.0f),
      maxDepthMeters_(maxDepthMeters),
      rawAdc_(0),
      voltage_(0.0f),
      depthMeters_(0.0f),
      status_(NOT_READY),
      ready_(false) {
    updateScale();
}

bool DepthSensor::begin() {
    if (!ads_.begin(i2cAddress_, wire_)) {
        ready_ = false;
        status_ = NOT_READY;
        return false;
    }

    ads_.setGain(GAIN_ONE);

    ready_ = true;
    status_ = OK;
    return true;
}

bool DepthSensor::update() {
    if (!ready_) {
        status_ = NOT_READY;
        return false;
    }

    rawAdc_ = ads_.readADC_SingleEnded(channel_);
    voltage_ = rawAdc_ * ADC_TO_VOLT;
    depthMeters_ = calcDepthFromVoltage(voltage_);
    status_ = evaluateStatus(voltage_);

    return true;
}

void DepthSensor::setCalibration(float zeroVoltage, float calVoltage, float calDepthMeters) {
    zeroVoltage_ = zeroVoltage;
    calVoltage_ = calVoltage;
    calDepthMeters_ = calDepthMeters;
    updateScale();
}

void DepthSensor::setMaxDepth(float maxDepthMeters) {
    if (maxDepthMeters > 0.0f) {
        maxDepthMeters_ = maxDepthMeters;
    }
}

int16_t DepthSensor::rawAdc() const {
    return rawAdc_;
}

float DepthSensor::voltage() const {
    return voltage_;
}

float DepthSensor::depthMeters() const {
    return depthMeters_;
}

float DepthSensor::depthCentimeters() const {
    return depthMeters_ * 100.0f;
}

DepthSensor::Status DepthSensor::status() const {
    return status_;
}

bool DepthSensor::isReady() const {
    return ready_;
}

const char* DepthSensor::statusToString(Status s) {
    switch (s) {
        case OK: return "OK";
        case NOT_READY: return "NOT_READY";
        case DISCONNECTED: return "DISCONNECTED";
        case OUT_OF_RANGE_LOW: return "OUT_OF_RANGE_LOW";
        case OUT_OF_RANGE_HIGH: return "OUT_OF_RANGE_HIGH";
        default: return "UNKNOWN";
    }
}

void DepthSensor::updateScale() {
    if (calDepthMeters_ <= 0.0f || calVoltage_ <= zeroVoltage_) {
        voltsPerMeter_ = 0.0f;
        return;
    }

    voltsPerMeter_ = (calVoltage_ - zeroVoltage_) / calDepthMeters_;
}

float DepthSensor::calcDepthFromVoltage(float voltage) const {
    if (voltsPerMeter_ <= 0.0f) {
        return 0.0f;
    }

    float depth = (voltage - zeroVoltage_) / voltsPerMeter_;

    if (depth < 0.0f) {
        depth = 0.0f;
    }

    if (depth > maxDepthMeters_) {
        depth = maxDepthMeters_;
    }

    return depth;
}

DepthSensor::Status DepthSensor::evaluateStatus(float voltage) const {
    if (!ready_) {
        return NOT_READY;
    }

    // These are simple voltage-domain checks, since we are intentionally
    // removing the current-domain step from the software model.
    if (voltage < 0.10f) {
        return DISCONNECTED;
    }

    if (voltage < (zeroVoltage_ - 0.05f)) {
        return OUT_OF_RANGE_LOW;
    }

    // 4.095 V is the nominal full-scale input limit at GAIN_ONE
    if (voltage > 4.095f) {
        return OUT_OF_RANGE_HIGH;
    }

    return OK;
}