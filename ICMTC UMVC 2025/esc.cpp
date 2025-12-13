#include "config.h"
#include <esc.h>


void ESCChannel::begin() {
    driver.setPeriodHertz(freq);
    driver.attach(channel_pin, ESC_MAX_CCW, ESC_MAX_CC);
    driver.writeMicroseconds(set_signal);
}

void ESCChannel::drive(float speed) {
  if(is_unidirectional) driver.writeMicroseconds(ESC_MAX_CCW + abs(speed) * 800); 
  else driver.writeMicroseconds(ESC_STOP + mid_diff * speed);
}