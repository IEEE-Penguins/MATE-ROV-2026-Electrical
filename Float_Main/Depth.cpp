#include "Depth.h"

bool Depth::waitReady(uint32_t timeoutUs) {
  uint32_t startUs = micros();
  while (digitalRead(doutPin)) {
    if ((uint32_t)(micros() - startUs) >= timeoutUs) {
      return false;
    }
    delayMicroseconds(5);
  }
  return true;
}

long Depth::readRaw() {
  long count = 0;

  if (!waitReady(50000U)) {
    return val;
  }

  for (uint8_t i = 0; i < 24; i++) {
    digitalWrite(sckPin, HIGH);
    count = count << 1;
    digitalWrite(sckPin, LOW);

    if (digitalRead(doutPin)) {
      count++;
    }
  }

  digitalWrite(sckPin, HIGH);
  digitalWrite(sckPin, LOW);

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
  digitalWrite(sckPin, LOW);
}

void Depth::calibrate(uint8_t samples) {
  if (samples == 0) {
    samples = 1;
  }

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
  if (samples == 0) {
    samples = 1;
  }
  numSamples = samples;
}

long Depth::read() {
  uint8_t sampleCount = (numSamples == 0) ? 1 : numSamples;
  long sum = 0;
  for (uint8_t i = 0; i < sampleCount; i++) {
    sum += readRaw();
  }
  return sum / sampleCount;
}

float Depth::getPressure() {
  long diff = read() - offset;
  return diff * calibrationFactor;
}

float Depth::getDepthMeters() {
  return getPressure() / 9810.0f;
}

float Depth::getDepthCM() {
  return getDepthMeters() * 100.0f;
}
