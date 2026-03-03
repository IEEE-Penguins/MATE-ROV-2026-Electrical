#include "MPU.h"

MPU6050::MPU6050(TwoWire &w) {
    wire = &w;
}

bool MPU6050::begin() {

    wire->beginTransmission(MPU6050_ADDR);
    if (wire->endTransmission() != 0)
        return false;

    writeRegister(MPU6050_PWR_MGMT_1, 0x00);
    writeRegister(MPU6050_SMPLRT_DIV, 0x00);
    writeRegister(MPU6050_CONFIG, 0x00);
    writeRegister(MPU6050_GYRO_CONFIG, 0x08);   // ±500 dps
    writeRegister(MPU6050_ACCEL_CONFIG, 0x00);  // ±2g

    delay(100);

    update();

    kalmanRoll.setAngle(_rollAcc);
    kalmanPitch.setAngle(_pitchAcc);

    _yaw = 0;
    _prevMicros = micros();

    return true;
}

void MPU6050::writeRegister(uint8_t reg, uint8_t data) {
    wire->beginTransmission(MPU6050_ADDR);
    wire->write(reg);
    wire->write(data);
    wire->endTransmission();
}

void MPU6050::readBurst() {
    wire->beginTransmission(MPU6050_ADDR);
    wire->write(0x3B);
    wire->endTransmission(false);
    wire->requestFrom(MPU6050_ADDR, 14);

    _rawAccX  = wire->read() << 8 | wire->read();
    _rawAccY  = wire->read() << 8 | wire->read();
    _rawAccZ  = wire->read() << 8 | wire->read();
    _rawTemp  = wire->read() << 8 | wire->read();
    _rawGyroX = wire->read() << 8 | wire->read();
    _rawGyroY = wire->read() << 8 | wire->read();
    _rawGyroZ = wire->read() << 8 | wire->read();
}

void MPU6050::update() {

    uint32_t now = micros();
    _dt = (now - _prevMicros) * 1e-6f;
    _prevMicros = now;

    readBurst();

    _accX = _rawAccX / 16384.0f;
    _accY = _rawAccY / 16384.0f;
    _accZ = _rawAccZ / 16384.0f;

    _gyroX = (_rawGyroX / 65.5f) - _gyroXoffset;
    _gyroY = (_rawGyroY / 65.5f) - _gyroYoffset;
    _gyroZ = (_rawGyroZ / 65.5f) - _gyroZoffset;

    _temp = (_rawTemp / 340.0f) + 36.53f;

    // Standard roll & pitch from accelerometer
    _rollAcc  = atan2(_accY, _accZ) * 180.0f / PI;
    _pitchAcc = atan2(-_accX, sqrt(_accY * _accY + _accZ * _accZ)) * 180.0f / PI;

    _roll  = kalmanRoll.getAngle(_rollAcc, _gyroX, _dt);
    _pitch = kalmanPitch.getAngle(_pitchAcc, _gyroY, _dt);

    _yaw += _gyroZ * _dt;
}

void MPU6050::calibrateGyro(uint16_t samples) {

    float sumX = 0, sumY = 0, sumZ = 0;

    for (uint16_t i = 0; i < samples; i++) {
        readBurst();
        sumX += _rawGyroX / 65.5f;
        sumY += _rawGyroY / 65.5f;
        sumZ += _rawGyroZ / 65.5f;
        delay(2);
    }

    _gyroXoffset = sumX / samples;
    _gyroYoffset = sumY / samples;
    _gyroZoffset = sumZ / samples;
}

/* ================= Kalman ================= */

MPU6050::Kalman::Kalman() {
    Q_angle = 0.001f;
    Q_gyro = 0.003f;
    R_measure = 0.03f;

    angle = 0;
    bias = 0;

    P[0][0] = P[0][1] = P[1][0] = P[1][1] = 0;
}

void MPU6050::Kalman::setAngle(float a) {
    angle = a;
}

float MPU6050::Kalman::getAngle(float newAngle, float newRate, float dt) {

    float rate = newRate - bias;
    angle += dt * rate;

    P[0][0] += dt * (dt * P[1][1] - P[0][1] - P[1][0] + Q_angle);
    P[0][1] -= dt * P[1][1];
    P[1][0] -= dt * P[1][1];
    P[1][1] += Q_gyro * dt;

    float S = P[0][0] + R_measure;
    float K0 = P[0][0] / S;
    float K1 = P[1][0] / S;

    float y = newAngle - angle;

    angle += K0 * y;
    bias  += K1 * y;

    float P00_temp = P[0][0];
    float P01_temp = P[0][1];

    P[0][0] -= K0 * P00_temp;
    P[0][1] -= K0 * P01_temp;
    P[1][0] -= K1 * P00_temp;
    P[1][1] -= K1 * P01_temp;

    return angle;
}

/* ========== Getters ========== */

int16_t MPU6050::rawAccX(){ return _rawAccX; }
int16_t MPU6050::rawAccY(){ return _rawAccY; }
int16_t MPU6050::rawAccZ(){ return _rawAccZ; }
int16_t MPU6050::rawGyroX(){ return _rawGyroX; }
int16_t MPU6050::rawGyroY(){ return _rawGyroY; }
int16_t MPU6050::rawGyroZ(){ return _rawGyroZ; }

float MPU6050::accX(){ return _accX; }
float MPU6050::accY(){ return _accY; }
float MPU6050::accZ(){ return _accZ; }

float MPU6050::gyroX(){ return _gyroX; }
float MPU6050::gyroY(){ return _gyroY; }
float MPU6050::gyroZ(){ return _gyroZ; }

float MPU6050::temperature(){ return _temp; }

float MPU6050::roll(){ return _roll; }
float MPU6050::pitch(){ return _pitch; }
float MPU6050::yaw(){ return _yaw; }

float MPU6050::rollAcc(){ return _rollAcc; }
float MPU6050::pitchAcc(){ return _pitchAcc; }

float MPU6050::dt(){ return _dt; }