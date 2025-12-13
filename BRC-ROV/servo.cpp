#include <servo.h>

void ServoDriver::begin() {
  driver.attach(pin);
            
  if(is_openloop) {
    driver.write(90);
  }
  else {
    current_angle = constrain(set_angle, 0, 180);
    driver.write(set_angle); 
  }
}



void ServoDriver::drive(int dir) {

  if(is_openloop) {
    driver.write(90 + 90* dir);
  }
  else if(dir != 0) {
    current_angle = constrain(current_angle + dir * SERVO_STEP, 0, 180);
    driver.write(current_angle);
  }
}
