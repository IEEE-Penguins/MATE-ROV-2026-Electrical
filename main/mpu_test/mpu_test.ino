#include <Wire.h>
#include "mpu.h"

MPU6050 imu(Wire);

void setup() {
    Serial.begin(115200);

    Wire.begin(21, 22);
    Wire.setClock(400000);

    if (!imu.begin()) {
        Serial.println("MPU6050 not detected.");
        while (1);
    }

    Serial.println("MPU6050 connected.");
    Serial.println("Calibrating gyro... Keep IMU still.");
    imu.calibrateGyro();
    Serial.println("Calibration complete.");
}

void loop() {

    imu.update();

    Serial.print("Roll: ");
    Serial.print(imu.roll(), 2);
    Serial.print(" | Pitch: ");
    Serial.print(imu.pitch(), 2);
    Serial.print(" | Yaw: ");
    Serial.print(imu.yaw(), 2);
    Serial.print(" | Temp: ");
    Serial.print(imu.temperature(), 2);
    Serial.println(" C");

    delay(10);
}