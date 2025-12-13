#ifndef JSN_H
#define JSN_H

#include <Arduino.h>

typedef struct {
    uint8_t triggerPin;
    uint8_t echoPin;
    unsigned long timeout;  // in microseconds
} JSNSensor;

void JSN_init(JSNSensor* sensor, uint8_t trigPin, uint8_t echoPin, unsigned long timeout);
float JSN_getDistance(JSNSensor* sensor);  // returns distance in cm

#endif
