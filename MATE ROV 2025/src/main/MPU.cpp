
#include "MPU.h"

void setup_MPU(int SCL_pin, int SDA_pin, Adafruit_MPU6050 &mpu_object) {

  Wire.setSDA(SDA_pin);
  Wire.setSCL(SCL_pin);
  Wire.begin();
  
  if (!mpu_object.begin()) {  
    Serial.println("Failed to initialize MPU6050!");
    while (1); //> Stop execution if MPU6050 fails
  }

  mpu_object.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu_object.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu_object.setFilterBandwidth(MPU6050_BAND_5_HZ);

}

MPU_Readings get_MPU_readings(Adafruit_MPU6050 &mpu_object) {
  sensors_event_t a, g, temp;
  mpu_object.getEvent(&a, &g, &temp);

  MPU_Readings val;

  val.acc_x = a.acceleration.x; //> m/s^2
  val.acc_y = a.acceleration.y; //> m/s^2
  val.acc_z = a.acceleration.z; //> m/s^2
  val.gyro_x = g.gyro.x; //> rad/s
  val.gyro_y = g.gyro.y; //> rad/s
  val.gyro_z = g.gyro.z; //> rad/s
  val.temp = temp.temperature; //> degC

  return val;

}
