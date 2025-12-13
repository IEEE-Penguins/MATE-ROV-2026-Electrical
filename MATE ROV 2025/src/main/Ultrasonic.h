
#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <Arduino.h>

float get_distance(int rx_trig, int tx_echo, float sound_speed);

#endif