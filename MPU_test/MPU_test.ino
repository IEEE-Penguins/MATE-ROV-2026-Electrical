#include "PCA.h"
#include "MPU.h"

PCA pwm;
Motor thrusters[] = {{THRUSTER_TYPE, 0},
                     {THRUSTER_TYPE, 1},
                     {THRUSTER_TYPE, 2}, 
                     {THRUSTER_TYPE, 3},
                     {THRUSTER_TYPE, 4},
                     {THRUSTER_TYPE, 5}};

#define thruster_size sizeof(thrusters) / sizeof(thrusters[0])

MPU6050 mpu6050(Wire);

void setup() {

  Serial.begin(115200);
  pwm.beginPCA(thrusters, thruster_size);
  
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  if (Serial.available() > 0) {
    // Read numeric input from Serial Monitor
    int usInput = Serial.parseInt();

    // Check for a valid ESC range (usually 1000us to 2000us)
    if (usInput >= 10 && usInput <= 5000) {
      Serial.print("Setting Pulse to: ");
      Serial.print(usInput);
      Serial.println(" us");

      // Use the library's built-in microsecond function
    for (byte i = 0; i < thruster_size; i++)
      pwm.pwmDrive(thrusters[i].type, usInput, thrusters[i].channel);
    } 
    else if (usInput != 0) {
      Serial.println("Invalid range! Please enter a value between 1000 and 2000.");
    }        
  }
}
 

void loop() {

  mpu6050.update();
  Serial.print("angleX : ");
  Serial.print(mpu6050.getAngleX());
  Serial.print("\tangleY : ");
  Serial.print(mpu6050.getAngleY());
  Serial.print("\tangleZ : ");
  Serial.println(mpu6050.getAngleZ());

  delay(500);
}