#include "mpu.h"
#include <Wire.h>
#include <DFRobot_BMI160.h>

DFRobot_BMI160 bmi160;
MPUValues mpu_data;

void MPU::begin() {
  Wire.begin();

  if (bmi160.I2cInit() != BMI160_OK) {
    while (1);  // Lock if initialization fails
  }
}

MPUValues MPU::read() {
  bmi160.getAccelData(accel);
  bmi160.getGyroData(gyro);

  float raw_roll  = gyro[1] / 131.0;     // Gyro Y-axis
  float raw_pitch = gyro[0] / 131.0;     // Gyro X-axis

  filtered_roll  = smooth(raw_roll,  filtered_roll, 0.4, 2.0);
  filtered_pitch = smooth(raw_pitch, filtered_pitch, 0.4, 2.0);

  mpu_data.acc_x = accel[0] / 16384.0;
  mpu_data.acc_y = accel[1] / 16384.0;
  mpu_data.acc_z = accel[2] / 16384.0;

  mpu_data.gyro_x = gyro[0];
  mpu_data.gyro_y = gyro[1];
  mpu_data.gyro_z = gyro[2];

  mpu_data.angle_x = filtered_roll;
  mpu_data.angle_y = filtered_pitch;

  return mpu_data;
}

float MPU::smooth(float current, float previous, float factor, float threshold) {
  float diff = abs(current - previous);
  float adjustedFactor = (diff > threshold) ? factor : factor * 0.5;
  return previous + adjustedFactor * (current - previous);
}
