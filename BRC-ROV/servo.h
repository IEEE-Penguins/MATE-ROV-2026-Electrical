#ifndef SERVO_H
#define SERVO_H

#include <config.h>
#include <Arduino.h>
#include <ESP32Servo.h>

class ServoDriver {
  private:
    Servo driver;
    int current_angle = 0;
    int pin;
    int set_angle;
    bool is_openloop;

  public:

    ServoDriver(int pin, int set_angle = 0, int is_openloop = false) {
      this->pin = pin;
      this->set_angle = set_angle;
      this->is_openloop = is_openloop;
    }

    void begin();
    void drive(int dir);

};

#endif
