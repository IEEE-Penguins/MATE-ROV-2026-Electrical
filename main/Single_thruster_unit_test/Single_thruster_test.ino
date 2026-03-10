#include "ESC.h"

#define motor_bin_1 2
#define motor_bin_2 4
#define motor_bin_3 16
#define motor_bin_4 17
#define motor_bin_5 5
#define motor_bin_6 15

ESCChannel Motor1(motor_bin_1, ESC_STOP);
ESCChannel Motor2(motor_bin_2, ESC_STOP);
ESCChannel Motor3(motor_bin_3, ESC_STOP);
ESCChannel Motor4(motor_bin_4, ESC_STOP);
ESCChannel Motor5(motor_bin_5, ESC_STOP);
ESCChannel Motor6(motor_bin_6, ESC_STOP);

void setup() {
  Serial.begin(115200);
}

void loop() {
  Motor1.drive(1300);
  Motor2.drive(1300);
  Motor3.drive(1300);
  Motor4.drive(1300);
  Motor5.drive(1300);
  Motor6.drive(1300);
  delay(3000);
  Motor1.stop();
  Motor2.stop();
  Motor3.stop();
  Motor4.stop();
  Motor5.stop();
  Motor6.stop();
  delay(3000);
  Motor1.drive(1700);
  Motor2.drive(1700);
  Motor3.drive(1700);
  Motor4.drive(1700);
  Motor5.drive(1700);
  Motor6.drive(1700);
  delay(3000);
  Motor1.stop();
  Motor2.stop();
  Motor3.stop();
  Motor4.stop();
  Motor5.stop();
  Motor6.stop();
  delay(3000);
}
