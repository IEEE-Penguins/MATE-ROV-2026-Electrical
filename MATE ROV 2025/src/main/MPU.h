
#ifndef MPU_H
#define MPU_H

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

typedef struct {
  float acc_x;
  float acc_y;
  float acc_z;
  float gyro_x;
  float gyro_y;
  float gyro_z;
  float temp;
} MPU_Readings;

void setup_MPU(int SCL_pin, int SDA_pin, Adafruit_MPU6050 &mpu_object);
MPU_Readings get_MPU_readings(Adafruit_MPU6050 &mpu_object);

#endif