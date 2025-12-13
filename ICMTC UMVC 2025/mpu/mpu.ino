#include <mpu.h>

MPU mpu;

void setup() {
  Wire.begin();
  mpu.begin();
  Serial.begin(115200);
}

void loop() {

  delay(50);
  MPUValues values = mpu.read();
  
  Serial.print("ax: "); Serial.print(values.acc_x); Serial.print(", ");
  Serial.print("ay: "); Serial.print(values.acc_y); Serial.print(", ");
  Serial.print("az: "); Serial.println(values.acc_z);

  Serial.print("gx: "); Serial.print(values.gyro_x); Serial.print(", ");
  Serial.print("gy: "); Serial.print(values.gyro_y); Serial.print(", ");
  Serial.print("gz: "); Serial.println(values.gyro_z);

  Serial.print("temp: "); Serial.println(values.temp);

  delay(50); 

}