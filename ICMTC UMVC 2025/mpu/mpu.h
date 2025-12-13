#ifndef MPU_H
#define MPU_H

#include <../config.h>
#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <Kalman.h>

// MPU6050 register addresses
#define MPU6050_ADDR 0x68
#define REG_PWR_MGMT_1 0x6B
#define REG_ACCEL_XOUT_H 0x3B
#define REG_TEMP_OUT_H 0x41
#define REG_ACCEL_CONFIG 0x1C
#define REG_GYRO_CONFIG 0x1B

// Thresholds for accident detection
#define ACC_THRESHOLD 2.5   // Acceleration in g
#define GYRO_THRESHOLD 300  // Angular velocity in °/s

typedef struct {
  float acc_x;
  float acc_y;
  float acc_z;
  float gyro_x;
  float gyro_y;
  float gyro_z;
  float temp;
} MPUValues;

class MPU {
  public:
    void begin();
    MPUValues read();
    
  private:
    void mpu_read();
    void processSensorData();
    
    // MPU6050 variables
    int16_t AcX, AcY, AcZ, GyX, GyY, GyZ, TempRaw;
    float accelX, accelY, accelZ, gyroX, gyroY, gyroZ;
    float angleX, angleY, temperature, dt = 0.01;

    // Kalman filter objects
    Kalman kalmanAccX, kalmanAccY, kalmanAccZ;
    Kalman kalmanGyroX, kalmanGyroY, kalmanGyroZ;
    Kalman kalmanAngleX, kalmanAngleY, kalmanTemp;

};

#endif  