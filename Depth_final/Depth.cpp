#include "Depth.h"

long Depth::readRaw() {
    long count = 0;
    
    if (digitalRead(doutPin)) return val;
    // while (digitalRead(doutPin));

    for (uint8_t i = 0; i < 24; i++) {
        digitalWrite(sckPin, HIGH);
        count = count << 1;
        digitalWrite(sckPin, LOW);

        if (digitalRead(doutPin)) count++;
    }

    digitalWrite(sckPin, HIGH);
    digitalWrite(sckPin, LOW);

    // sign extend
    if (count & 0x800000) {
        count |= ~0xffffff;
    }

    val = count;
    return count;
}

void Depth::begin(uint8_t dout_pin, uint8_t sck_pin) {
    doutPin = dout_pin;
    sckPin = sck_pin;
    pinMode(doutPin, INPUT);
    pinMode(sckPin, OUTPUT);
}

void Depth::calibrate(uint8_t samples) {
    long sum = 0;
    for (uint8_t i = 0; i < samples; i++) {
        sum += readRaw();
    }
    offset = sum / samples;
}

void Depth::setCalibrationFactor(float factor) {
    calibrationFactor = factor;
}

void Depth::setSamples(uint8_t samples) {
    numSamples = samples;
}

long Depth::read() {
    long sum = 0;
    for (uint8_t i = 0; i < numSamples; i++) {
        sum += readRaw();
    }
    return sum / numSamples;
}

float Depth::getPressure() {            // Pascal
    long diff = read() - offset;
    return diff * calibrationFactor;
}

float Depth::getDepthMeters() {         // meter
    return getPressure() / 9810.0;
}

float Depth::getDepthCM() {             // cm
    return getDepthMeters() * 100.0;
}    