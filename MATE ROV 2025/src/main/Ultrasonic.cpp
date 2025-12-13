
#include "Ultrasonic.h"


float get_distance(int rx_trig, int tx_echo, float sound_speed) {

  digitalWrite(rx_trig, LOW);
  delayMicroseconds(2);

  digitalWrite(rx_trig, HIGH);
  delayMicroseconds(20);
  digitalWrite(rx_trig, LOW);

  pinMode(rx_trig, INPUT);
  long duration = pulseIn(tx_echo, HIGH);
  int distance = duration * sound_speed / 20000;

  pinMode(rx_trig, OUTPUT);

  return distance;

}