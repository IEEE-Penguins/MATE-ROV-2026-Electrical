#include <Wire.h>
#include "ESC.h"
#include "MPU.h"

#define MOTOR_PIN 15 

// --- Embedded Kalman Filter Class ---
class MyKalman {
public:
    MyKalman() {
        Q_angle = 0.001; Q_gyro = 0.003; R_measure = 0.03;
        angle = 0; bias = 0;
        P[0][0] = 0; P[0][1] = 0; P[1][0] = 0; P[1][1] = 0;
    }
    double getAngle(double newAngle, double newRate, double dt) {
        double rate = newRate - bias;
        angle += dt * rate;
        P[0][0] += dt * (dt * P[1][1] - P[0][1] - P[1][0] + Q_angle);
        P[0][1] -= dt * P[1][1];
        P[1][0] -= dt * P[1][1];
        P[1][1] += Q_gyro * dt;
        double S = P[0][0] + R_measure;
        double K[2];
        K[0] = P[0][0] / S; K[1] = P[1][0] / S;
        double y = newAngle - angle;
        angle += K[0] * y; bias += K[1] * y;
        double P00_temp = P[0][0]; double P01_temp = P[0][1];
        P[0][0] -= K[0] * P00_temp; P[0][1] -= K[0] * P01_temp;
        P[1][0] -= K[1] * P00_temp; P[1][1] -= K[1] * P01_temp;
        return angle;
    };
    void setAngle(double a) { angle = a; };
private:
    double Q_angle, Q_gyro, R_measure;
    double angle, bias;
    double P[2][2];
};

// Objects
ESCChannel Motor(MOTOR_PIN, ESC_STOP);
MPU6050 mpu6050(Wire);
MyKalman kalmanX; 
MyKalman kalmanY;

// Variables
double kalAngleX, kalAngleY;
uint32_t timer;
int userSpeed = 1000;
bool mpuReady = false;

void setup() {
  Serial.begin(115200);

  // 1. ARM THE ESC IMMEDIATELY
  Serial.println("ESC Armed with 1000us signal.");

  // 2. INITIALIZE I2C & MPU
  Wire.begin(21, 22);
  Wire.setClock(400000); // Fast I2C for Kalman
  
  Serial.println("Checking MPU6050...");
  Wire.beginTransmission(0x68);
  if (Wire.endTransmission() == 0) {
    mpu6050.begin();
    Serial.println("MPU found! Calibrating... DO NOT MOVE.");
    mpu6050.calcGyroOffsets(true);
    
    // Set initial Kalman state
    mpu6050.update();
    kalmanX.setAngle(mpu6050.getAngleX()); 
    kalmanY.setAngle(mpu6050.getAngleY());
    mpuReady = true;
  } else {
    Serial.println("MPU NOT FOUND! Motor will run in 'Raw' mode.");
  }

  // 3. GET USER SPEED
  Serial.println("------------------------------------------");
  Serial.println("ENTER MOTOR SPEED (1000-2000) AND PRESS ENTER:");
  while (Serial.available() == 0) { delay(10); }
  userSpeed = Serial.parseInt();
  
  Serial.print("Starting at speed: "); Serial.println(userSpeed);
  
  timer = micros();
  Serial.println("SYSTEM STARTING...");
}

void loop() {
  // A. UPDATE MOTOR (Hardware PWM remains stable even if I2C lags)
  Motor.drive(userSpeed);

  // B. PROCESS SENSOR DATA (Only if MPU was found)
  if (mpuReady) {
    mpu6050.update();
    
    double dt = (double)(micros() - timer) / 1000000; 
    timer = micros();

    kalAngleX = kalmanX.getAngle(mpu6050.getAngleX(), mpu6050.getGyroX(), dt);
    kalAngleY = kalmanY.getAngle(mpu6050.getAngleY(), mpu6050.getGyroY(), dt);

    // Serial Plotter format
    Serial.print("KalmanX:"); Serial.print(kalAngleX);
    Serial.print("      ");
    Serial.print("KalmanY:"); Serial.print(kalAngleY);
    Serial.print("      ");
  } else {
    Serial.print("MPU_ERROR:0,");
  }

  Serial.print("MotorSpeed:"); Serial.println(userSpeed);
  
  // Minimal delay to prevent Serial overflow but keep Kalman loop fast
  delay(2); 
}