#ifndef MPU_H
#define MPU_H

#include <Arduino.h>
#include <Wire.h>

#define MPU6050_ADDR         0x68
#define MPU6050_SMPLRT_DIV   0x19
#define MPU6050_CONFIG       0x1A
#define MPU6050_GYRO_CONFIG  0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_PWR_MGMT_1   0x6B
#define MPU6050_WHO_AM_I     0x75

// Magnetometer address used by the supplied combined MPU + MAG reference module.
#define QMC5883_ADDR         0x2C

class MPU6050 {
public:

    MPU6050(TwoWire &w);

    bool begin();
    void update();

    // Calibration
    void calibrateGyro(uint16_t samples = 3000);

    // Raw access
    int16_t rawAccX(), rawAccY(), rawAccZ();
    int16_t rawGyroX(), rawGyroY(), rawGyroZ();

    // Physical units
    float accX(), accY(), accZ();        // g
    float gyroX(), gyroY(), gyroZ();     // deg/s
    float temperature();                 // °C

    // Angles (degrees)
    float roll();        // Kalman filtered
    float pitch();       // Kalman filtered
    float yaw();         // Absolute magnetic yaw/heading, degrees

    float rollAcc();     // From accelerometer only
    float pitchAcc();    // From accelerometer only

    float dt();          // seconds

private:

    void writeRegister(uint8_t reg, uint8_t data);
    void readBurst();

    // Magnetometer helpers imported from the supplied reference module.
    bool setupQMC();
    bool readQMC();
    void computeMagYaw();
    static float wrapAngle180(float angleDeg);

    // Embedded Kalman Filter
    class Kalman {
    public:
        Kalman();
        void setAngle(float angle);
        float getAngle(float newAngle, float newRate, float dt);
    private:
        float Q_angle, Q_gyro, R_measure;
        float angle, bias;
        float P[2][2];
    };

    TwoWire *wire;

    // Raw data
    int16_t _rawAccX, _rawAccY, _rawAccZ;
    int16_t _rawTemp;
    int16_t _rawGyroX, _rawGyroY, _rawGyroZ;

    // Scaled
    float _accX, _accY, _accZ;
    float _gyroX, _gyroY, _gyroZ;
    float _temp;

    // Offsets
    float _gyroXoffset = 0;
    float _gyroYoffset = 0;
    float _gyroZoffset = 0;

    // Magnetometer state/calibration.
    bool _magReady = false;
    float _magX = 0.0f;
    float _magY = 0.0f;
    float _magZ = 0.0f;

    float _magOffsetX = 267.50f;
    float _magOffsetY = 222.00f;
    float _magOffsetZ = 200.50f;

    float _magScaleX = 1.053206f;
    float _magScaleY = 1.000000f;
    float _magScaleZ = 0.951911f;

    // Angles
    float _rollAcc, _pitchAcc;
    float _roll, _pitch, _yaw;

    // Timing
    uint32_t _prevMicros;
    float _dt;

    Kalman kalmanRoll;
    Kalman kalmanPitch;
};

#endif