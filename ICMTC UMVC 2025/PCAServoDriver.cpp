#include <PCAServoDriver.h>

void ServosController::begin() {
  driver.begin();
  driver.setPWMFreq(freq);
}

void ServosController::reset_closed_loop(int channel) {
  driver.setPWM(channel, 0, SERVO_MIN);
}

void ServosController::reset_open_loop(int channel) {
  driver.setPWM(channel, 0, 350);
}

void ServosController::closed_loop_drive(int channel, int dir) {
  if(dir == 0) return;
  int current_angle = driver.getPWM(channel, true);
  int new_angle = constrain(current_angle += dir * SERVO_STEP, SERVO_MIN, SERVO_MAX);
  driver.setPWM(channel, 0, new_angle);
}


void ServosController::open_loop_drive(int channel, int dir) {
  if(dir == 1) driver.setPWM(channel, 0, 250);
  else if(dir == -1)  driver.setPWM(channel, 0, 450);
  else driver.setPWM(channel, 0, 350);
}
