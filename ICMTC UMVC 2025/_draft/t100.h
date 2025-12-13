#ifndef T100_H
#define T100_H

#include <Arduino.h>
#include <ESP32Servo.h>

class ESCController {
  public:
    void init();
    void set(int index, int value, bool forward = true);
    
  private:
    static const int NUM_MOTORS = 8;
    const int motorPins[NUM_MOTORS] = {13, 12, 14, 27, 26, 25, 33, 32};
    Servo motors[NUM_MOTORS];

    const int MIN_PWM = 1100;
    const int MAX_PWM = 1900;
    const int NEUTRAL_PWM = 1500;
    const int MAX_ESC_INPUT = 255;
};

#endif  // T100_H
