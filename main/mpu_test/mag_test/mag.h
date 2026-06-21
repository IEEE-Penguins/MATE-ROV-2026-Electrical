#pragma once

#include <Arduino.h>
#include <Wire.h>

class MPU6050Kalman {
public:
  void begin();
  void update();

  float getPitch();
  float getYaw();

private:
  static const uint8_t MPU_ADDR = 0x68;
  static const uint8_t QMC_ADDR = 0x2C;

  float RateRoll, RatePitch, RateYaw;
  float RateCalibrationRoll = 0;
  float RateCalibrationPitch = 0;
  float RateCalibrationYaw = 0;

  float AccX, AccY, AccZ;
  float AngleRoll, AnglePitch;

  float MagX, MagY, MagZ;
  float AngleYaw;

  float RollOffset = 0;
  float PitchOffset = 0;
  float YawOffset = 0;

  float KalmanAngleRoll = 0;
  float KalmanUncertaintyAngleRoll = 4;

  float KalmanAnglePitch = 0;
  float KalmanUncertaintyAnglePitch = 4;

  float KalmanAngleYaw = 0;
  float KalmanUncertaintyAngleYaw = 4;

  float Kalman1DOutput[2] = {0, 0};

  uint32_t LoopTimer;

  bool yawWasGyroOnly = false;

  const float rollAngleGain = 1.10;
  const float pitchAngleGain = 1.10;

  const float yawGyroScale = 1.00;
  const float yawReliableTiltLimit = 45.0;
  const float yawMagUncertainty = 25 * 25;
  const float yawTurnRateLimit = 8.0;

  float magOffsetX = 267.50;
  float magOffsetY = 222.00;
  float magOffsetZ = 200.50;

  float magScaleX = 1.053206;
  float magScaleY = 1.000000;
  float magScaleZ = 0.951911;

  void setupMPU();
  void setupQMC();

  void readMPU();
  void readQMC();

  void computeAccelAngles();
  void computeYawAngle();

  void calibrateGyro();

  void kalman1D(float state, float uncertainty,
                float input, float measurement,
                float measurementUncertainty, float dt);

  void kalmanAngle1D(float state, float uncertainty,
                     float input, float measurement,
                     float measurementUncertainty, float dt);

  float wrapAngle(float angle);
};
