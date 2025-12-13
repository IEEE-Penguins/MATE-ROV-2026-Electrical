#include <mpu.h>

void MPU::begin() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(REG_PWR_MGMT_1);
  Wire.write(0x00); // Activate MPU6050
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(REG_ACCEL_CONFIG);
  Wire.write(0x00); // ±2g
  Wire.endTransmission(true);

  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(REG_GYRO_CONFIG);
  Wire.write(0x00); // ±250°/s
  Wire.endTransmission(true);
}

void MPU::mpu_read() {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(REG_ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 14, true);

  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  TempRaw = Wire.read() << 8 | Wire.read(); // Read temperature
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();
}

void MPU::processSensorData() {
  // Convert raw accelerometer data to g
  accelX = kalmanAccX.getAngle((float)AcX / 16384.0, 0, dt);
  accelY = kalmanAccY.getAngle((float)AcY / 16384.0, 0, dt);
  accelZ = kalmanAccZ.getAngle((float)AcZ / 16384.0, 0, dt);

  // Convert raw gyroscope data to °/s
  gyroX = kalmanGyroX.getAngle((float)GyX / 131.0, 0, dt);
  gyroY = kalmanGyroY.getAngle((float)GyY / 131.0, 0, dt);
  gyroZ = kalmanGyroZ.getAngle((float)GyZ / 131.0, 0, dt);

  // Calculate angles from accelerometer data
  float accelAngleX = atan(AcY / sqrt(pow(AcX, 2) + pow(AcZ, 2))) * 180 / PI;
  float accelAngleY = atan(-AcX / sqrt(pow(AcY, 2) + pow(AcZ, 2))) * 180 / PI;
  angleX = kalmanAngleX.getAngle(accelAngleX, gyroX, dt);
  angleY = kalmanAngleY.getAngle(accelAngleY, gyroY, dt);

  // Convert raw temperature to degrees Celsius and filter
  float rawTemperature = TempRaw / 340.0 + 36.53;
  temperature = kalmanTemp.getAngle(rawTemperature, 0, dt);
}

MPUValues MPU::read() {
  MPUValues values;
  mpu_read();            // Read data from MPU6050
  processSensorData();   // Process raw and filtered data
  values = {
    (float)AcX / 16384.0, //~~ accelX
    (float)AcY / 16384.0, //~~ accelY
    (float)AcZ / 16384.0, //~~ accelZ
    (float)GyX / 131.0,   //~~ gyroX
    (float)GyY / 131.0,   //~~ gyroY
    (float)GyZ / 131.0,   //~~ gyroZ
    temperature              
  };

  //~~ angleX
  //~~ angleY

  delay(50);
  return values;
}