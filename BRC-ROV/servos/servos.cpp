#include <servos.h>

void ServosController::begin() {
  driver.begin();
  driver.setPWMFreq(freq);
  for(int i = 0; i < SERVOS_CHANNELS_NUM; i++) driver.setPWM(i, 0, SERVO_MIN);
  delay(4000);
}

void ServosController::drive(int channel, int dir) {
  int current_angle = driver.getPWM(channel, true);
  int new_angle = constrain(current_angle += dir * SERVO_STEP, SERVO_MIN, SERVO_MAX);
  Serial.println(new_angle);
  driver.setPWM(channel, 0, new_angle);
  delay(50);
}

