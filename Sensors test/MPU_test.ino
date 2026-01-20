#include "PCA.h"
#include "MPU.h"
#include "depth.h"
#include "depthConfig.h"

PCA pwm;
Motor thrusters[] = {{THRUSTER_TYPE, 0},
                     {THRUSTER_TYPE, 1},
                     {THRUSTER_TYPE, 2}, 
                     {THRUSTER_TYPE, 3},
                     {THRUSTER_TYPE, 4},
                     {THRUSTER_TYPE, 5}};

#define thruster_size sizeof(thrusters) / sizeof(thrusters[0])

MPU6050 mpu6050(Wire);
DepthSensor depthSensor;

void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n=== ROV Control System ===");
  
  pwm.beginPCA(thrusters, thruster_size);
  Serial.println("Thrusters initialized");
  
  if (Serial.available() > 0) {
    int usInput = Serial.parseInt();
    
    if (usInput >= 10 && usInput <= 5000) {
      Serial.print("Setting Pulse to: ");
      Serial.print(usInput);
      Serial.println(" us");
      
      for (byte i = 0; i < thruster_size; i++) {
        pwm.pwmDrive(thrusters[i].type, usInput, thrusters[i].channel);
      }
    } 
    else if (usInput != 0) {
      Serial.println("Invalid range! Please enter a value between 10 and 5000.");
    }
  }

  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  Serial.println("MPU6050 initialized");
  
  if (!depthSensor.begin()) {
    Serial.println("Failed to initialize depth sensor!");
    #if ENABLE_STATUS_LED
      pinMode(STATUS_LED_PIN, OUTPUT);
      while (1) {
        digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
        delay(500);
      }
    #endif
  }
  
  #if ENABLE_STATUS_LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH);
  #endif
  
  depthSensor.printCalibration();
  Serial.println("Depth sensor initialized");
  
  Serial.println("\n=== System Ready ===");
  Serial.println("Enter pulse width (10-5000 us) to control thrusters");
}

void loop() {
  
  mpu6050.update();
  Serial.print("angleX: ");
  Serial.print(mpu6050.getAngleX());
  Serial.print("\tangleY: ");
  Serial.print(mpu6050.getAngleY());
  Serial.print("\tangleZ: ");
  Serial.print(mpu6050.getAngleZ());
  Serial.print("\t");
  
  depthSensor.readSensor();
  
  delay(UPDATE_INTERVAL);
}