#include "ServoMotor.h"

ServoMotor servo1(26);

short control[] = {1, 1, 1, 1, 1, -1, -1, -1, 1, -1, 1, 1, 1};

void setup()
{
    Serial.begin(115200);
    servo1.setAngle(90, SERVO_180);
}

void loop()
{
    for (byte i = 0; i < 13; i++) {
        servo1.controlAngle(control[i], SERVO_180);
    }
}