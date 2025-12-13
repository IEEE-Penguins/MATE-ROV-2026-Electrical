#include "t100.h"

void ESCController::init() {
  for (int i = 0; i < NUM_MOTORS; i++) {
    motors[i].attach(motorPins[i], 1000, 2000);
    motors[i].writeMicroseconds(NEUTRAL_PWM);
  }
}

void ESCController::set(int index, int value, bool forward) {
  if (index < 0 || index >= NUM_MOTORS) return;
  value = constrain(value, 0, MAX_ESC_INPUT);

  int pwm = forward
    ? map(value, 0, MAX_ESC_INPUT, NEUTRAL_PWM, MAX_PWM)
    : map(value, 0, MAX_ESC_INPUT, NEUTRAL_PWM, MIN_PWM);

  motors[index].writeMicroseconds(value > 0 ? pwm : NEUTRAL_PWM);
}
