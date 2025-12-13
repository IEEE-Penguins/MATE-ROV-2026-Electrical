#include "jsn.h"

void JSN_init(JSNSensor* sensor, uint8_t trigPin, uint8_t echoPin, unsigned long timeout) {
    sensor->triggerPin = trigPin;
    sensor->echoPin = echoPin;
    sensor->timeout = timeout;

    pinMode(sensor->triggerPin, OUTPUT);
    pinMode(sensor->echoPin, INPUT);
    digitalWrite(sensor->triggerPin, LOW);
}

float JSN_getDistance(JSNSensor* sensor) {
    // Send 10us trigger pulse
    digitalWrite(sensor->triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(sensor->triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(sensor->triggerPin, LOW);

    // Measure echo pulse
    unsigned long duration = pulseIn(sensor->echoPin, HIGH, sensor->timeout);
    if (duration == 0) return -1.0; // Timeout

    float distance = (duration * 0.0343) / 2.0;
    return distance;
}
