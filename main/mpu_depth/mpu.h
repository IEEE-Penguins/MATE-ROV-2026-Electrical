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
    float yaw();         // Integrated gyro Z

    float rollAcc();     // From accelerometer only
    float pitchAcc();    // From accelerometer only

    float dt();          // seconds

private:

    void writeRegister(uint8_t reg, uint8_t data);
    void readBurst();

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