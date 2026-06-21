#include "mag.h"

void MPU6050Kalman::begin() {
  Wire.begin();
  delay(250);

  setupMPU();
  setupQMC();

  calibrateGyro();

  readMPU();
  readQMC();
  computeAccelAngles();

  RollOffset = AngleRoll;
  PitchOffset = AnglePitch;

  KalmanAngleRoll = 0;
  KalmanAnglePitch = 0;

  computeYawAngle();
  YawOffset = AngleYaw;
  KalmanAngleYaw = 0;

  yawWasGyroOnly = false;

  LoopTimer = micros();
}

void MPU6050Kalman::update() {
  uint32_t currentTime = micros();
  float dt = (currentTime - LoopTimer) / 1000000.0f;
  LoopTimer = currentTime;

  if (dt <= 0 || dt > 0.08f) {
    dt = 0.004f; //Error Handling if esp قررت تعلق
  }

  readMPU();
  readQMC();

  RateRoll -= RateCalibrationRoll;
  RatePitch -= RateCalibrationPitch;
  RateYaw -= RateCalibrationYaw;

  RateYaw *= yawGyroScale; // عشان لو كان اقل من العادي بس طلع مظبوط (ابجورة)

  computeAccelAngles();

  // الزوايا بالنسبة لوضع البداية
  float rollMeas = wrapAngle(AngleRoll - RollOffset); 
  float pitchMeas = wrapAngle(AnglePitch - PitchOffset);

  //كالمن فلتر بيتعامل بالانحراف المعياري فبنربع قيمة الخطأ المتوقع
  //الخطأ المتوقع هنا 3 درجات
  float rollUncertainty = 3 * 3; 
  float pitchUncertainty = 3 * 3;
  
  // عشان لما ال ام بي يو بيميل بيخرف  فهنزود قيمة الخطأ المتوقعة علمحور التاني لو مال على غيره جامد
  if (abs(KalmanAnglePitch) > 65) rollUncertainty = 30 * 30;
  if (abs(KalmanAngleRoll) > 65) pitchUncertainty = 30 * 30;

  kalman1D(KalmanAngleRoll, KalmanUncertaintyAngleRoll,
           RateRoll, rollMeas, rollUncertainty, dt);

  KalmanAngleRoll = Kalman1DOutput[0];
  KalmanUncertaintyAngleRoll = Kalman1DOutput[1];

  kalman1D(KalmanAnglePitch, KalmanUncertaintyAnglePitch,
           RatePitch, pitchMeas, pitchUncertainty, dt);

  KalmanAnglePitch = Kalman1DOutput[0];
  KalmanUncertaintyAnglePitch = Kalman1DOutput[1];

  // حركة روشة عشان نعرف لو ال ام بي يو مايل اوي هل قراءات الماجنتوميتر نقدر نعتمد عليها ولالا
  bool tiltIsReliable =
    abs(KalmanAngleRoll) < yawReliableTiltLimit &&
    abs(KalmanAnglePitch) < yawReliableTiltLimit;

  // عشان نعرف بيلف بسرعة ولالا
  bool yawIsTurningFast = abs(RateYaw) > yawTurnRateLimit;

  //هنستخدم ال جايرو بس لو الميل كبير او بيلف بسرعة
  bool useGyroOnly = !tiltIsReliable || yawIsTurningFast;

  if (useGyroOnly) {
    KalmanAngleYaw = wrapAngle(KalmanAngleYaw + RateYaw * dt);
    // هنزود الخطأ مع الوقت عشان طول محنا ماشيين بالجايرو بس هيحصل دريفت فمش نعتمد عليه كتير
    KalmanUncertaintyAngleYaw += dt * dt * 4 * 4;
    yawWasGyroOnly = true;
  } else {
    computeYawAngle();

    if (yawWasGyroOnly) {
      // عشان الماجنتو مش يشد الزاوية جامد
      YawOffset = wrapAngle(AngleYaw - KalmanAngleYaw);
      yawWasGyroOnly = false;
    }

    float yawMeas = wrapAngle(AngleYaw - YawOffset);

    kalmanAngle1D(KalmanAngleYaw, KalmanUncertaintyAngleYaw,
                  RateYaw, yawMeas, yawMagUncertainty, dt);

    KalmanAngleYaw = wrapAngle(Kalman1DOutput[0]);
    KalmanUncertaintyAngleYaw = Kalman1DOutput[1];
  }

  while (micros() - currentTime < 4000); //الكود شغال 250 هرتز
}

float MPU6050Kalman::getPitch() {
  return KalmanAnglePitch;
}

float MPU6050Kalman::getYaw() {
  return KalmanAngleYaw;
}

void MPU6050Kalman::kalman1D(float state, float uncertainty,
                             float input, float measurement,
                             float measurementUncertainty, float dt) {
  state = state + dt * input;
  uncertainty = uncertainty + dt * dt * 4 * 4;

  //معادلة عشان اطلع اثق في مين وبنسبة ايه
  float gain = uncertainty / (uncertainty + measurementUncertainty);
  // التصحيح
  state = state + gain * (measurement - state);
  // نقلل الشك عشان قسنا
  uncertainty = (1 - gain) * uncertainty;

  Kalman1DOutput[0] = state;
  Kalman1DOutput[1] = uncertainty;
}
// For Yaw عشان هي الوحيدة الي بتلف 360 درجة
void MPU6050Kalman::kalmanAngle1D(float state, float uncertainty,
                                  float input, float measurement,
                                  float measurementUncertainty, float dt) {
  state = wrapAngle(state + dt * input);
  uncertainty = uncertainty + dt * dt * 4 * 4;

  float gain = uncertainty / (uncertainty + measurementUncertainty);
  float error = wrapAngle(measurement - state);

  state = wrapAngle(state + gain * error);
  uncertainty = (1 - gain) * uncertainty;

  Kalman1DOutput[0] = state;
  Kalman1DOutput[1] = uncertainty;
}

void MPU6050Kalman::setupMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); //Power Managment register
  Wire.write(0x00); // Wake up MPU
  Wire.endTransmission();

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1A); //Digital Low Pass Filter
  Wire.write(0x05);
  Wire.endTransmission();

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1C); // Accelorometer Range
  Wire.write(0x10); // ±8g
  Wire.endTransmission();

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B); //Gyro Range
  Wire.write(0x08); //±500 deg/s
  Wire.endTransmission();
}

void MPU6050Kalman::setupQMC() {
  Wire.beginTransmission(QMC_ADDR);
  Wire.write(0x0B); //Reset Period Register
  Wire.write(0x80);  
  Wire.endTransmission();
  delay(100);

  Wire.beginTransmission(QMC_ADDR);
  Wire.write(0x0B);
  Wire.write(0x08); // Start After reset
  Wire.endTransmission();
  delay(10);

  Wire.beginTransmission(QMC_ADDR);
  Wire.write(0x0A); // Control Register 1
  Wire.write(0x0D); // 1101 -> Mode, ODR, RNG, OSR
  Wire.endTransmission();
  delay(100);
}

void MPU6050Kalman::readMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); //ACCEL_XOUT_H
  Wire.endTransmission(false);

  Wire.requestFrom(MPU_ADDR, 14, true); //Read From MPU 14 bytes

  //High byte then Low Byte
  //Reading comes at 2 bytes so we shift first 8 and then get second 8
  int16_t AccXLSB = Wire.read() << 8 | Wire.read(); 
  int16_t AccYLSB = Wire.read() << 8 | Wire.read();
  int16_t AccZLSB = Wire.read() << 8 | Wire.read();

  Wire.read();
  Wire.read();

  int16_t GyroX = Wire.read() << 8 | Wire.read();
  int16_t GyroY = Wire.read() << 8 | Wire.read();
  int16_t GyroZ = Wire.read() << 8 | Wire.read();

  RateRoll  = (float)GyroX / 65.5f;
  RatePitch = -(float)GyroY / 65.5f;
  RateYaw   = -(float)GyroZ / 65.5f;

  AccX = (float)AccXLSB / 4096.0f;
  AccY = (float)AccYLSB / 4096.0f;
  AccZ = (float)AccZLSB / 4096.0f;
}

void MPU6050Kalman::readQMC() {
  Wire.beginTransmission(QMC_ADDR);
  Wire.write(0x0A); // Control register
  Wire.write(0x0E);
  Wire.endTransmission();

  delay(20);

  Wire.beginTransmission(QMC_ADDR);
  Wire.write(0x01); // Read From register 0x01
  Wire.endTransmission(false);

  Wire.requestFrom(QMC_ADDR, 6, true);

  if (Wire.available() < 6) return;

  int16_t rawY = Wire.read() | (Wire.read() << 8);
  int16_t rawX = Wire.read() | (Wire.read() << 8);
  int16_t rawZ = Wire.read() | (Wire.read() << 8);

  MagX = ((float)rawX - magOffsetX) * magScaleX;
  MagY = ((float)rawY - magOffsetY) * magScaleY;
  MagZ = ((float)rawZ - magOffsetZ) * magScaleZ;
}

void MPU6050Kalman::computeAccelAngles() {
  AngleRoll = -atan2(AccY, sqrt(AccX * AccX + AccZ * AccZ)) * 180.0f / PI;
  AnglePitch = atan2(AccX, sqrt(AccY * AccY + AccZ * AccZ)) * 180.0f / PI;
  
  // Correction عشان القرايات كانت اقل
  AngleRoll *= rollAngleGain; 
  AnglePitch *= pitchAngleGain;

  AngleRoll = constrain(AngleRoll, -90.0f, 90.0f);
  AnglePitch = constrain(AnglePitch, -90.0f, 90.0f);
}

void MPU6050Kalman::computeYawAngle() {
  float rollRad = KalmanAngleRoll * DEG_TO_RAD;
  float pitchRad = KalmanAnglePitch * DEG_TO_RAD;

  float mx = MagX;
  float my = MagY;
  float mz = MagZ;

  // Tilt Compensation
  float magXComp = mx * cos(pitchRad) + mz * sin(pitchRad);
  
  //Rotation Matrices for Pitch and Roll To Correct the tilt
  float magYComp =
    mx * sin(rollRad) * sin(pitchRad) +
    my * cos(rollRad) -
    mz * sin(rollRad) * cos(pitchRad);

  AngleYaw = atan2(-magYComp, magXComp) * RAD_TO_DEG;
  AngleYaw = wrapAngle(AngleYaw);
}

void MPU6050Kalman::calibrateGyro() {
  Serial.println("Keep MPU still. Calibrating gyro...");
  delay(2000);

  for (int i = 0; i < 2000; i++) {
    readMPU();

    RateCalibrationRoll += RateRoll;
    RateCalibrationPitch += RatePitch;
    RateCalibrationYaw += RateYaw;

    delay(1);
  }

  RateCalibrationRoll /= 2000.0f;
  RateCalibrationPitch /= 2000.0f;
  RateCalibrationYaw /= 2000.0f;

  Serial.println("Gyro calibration done.");
}

float MPU6050Kalman::wrapAngle(float angle) {
  while (angle > 180.0f) angle -= 360.0f;
  while (angle < -180.0f) angle += 360.0f;
  return angle;
}

  /*  Magnetometer
          ↓
      AngleYaw
          ↓
      طرح offset
          ↓
      yawMeas
          ↓
      Kalman Filter + Gyro
          ↓
      KalmanAngleYaw
  */
