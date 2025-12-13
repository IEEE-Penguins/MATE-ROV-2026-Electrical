#include <thrusters.h>

void ThrustersController::begin() {
  for(int i = 0; i < ESCs_CHANNELS_NUM; i++) {
    esc[i].setPeriodHertz(freq);
    esc[i].attach(pin[i], ESC_MAX_CCW, ESC_MAX_CC);
    esc[i].writeMicroseconds(ESC_MAX_CCW);
  }
  delay(3000);
}

void ThrustersController::drive(float speed_array[ESCs_CHANNELS_NUM]) {
  for(int i = 0; i < ESCs_CHANNELS_NUM; i++) {
    esc[i].writeMicroseconds(ESC_STOP + mid_diff * speed_array[i]);
  }
} 