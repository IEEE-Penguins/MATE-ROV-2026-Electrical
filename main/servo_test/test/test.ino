#include <Arduino.h>
#include <ESP32Servo.h>

static const unsigned long SERIAL_BAUD_RATE = 460800;
static const int SERVO_PIN = 33;

Servo testServo;

void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);

    testServo.setPeriodHertz(50);
    testServo.attach(SERVO_PIN, 500, 2500);

    Serial.println("Testing pin 33 directly");

    Serial.println("Move to 1500 us");
    testServo.writeMicroseconds(1500);
    delay(2000);

    Serial.println("Move to 1000 us");
    testServo.writeMicroseconds(1000);
    delay(2000);

    Serial.println("Move to 2000 us");
    testServo.writeMicroseconds(2000);
    delay(2000);

    Serial.println("Move to 500 us");
    testServo.writeMicroseconds(500);
    delay(2000);

    Serial.println("Move to 2500 us");
    testServo.writeMicroseconds(2500);
    delay(2000);
}

void loop()
{
}