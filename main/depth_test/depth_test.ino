#include <Wire.h>
#include "depth.h"

// App-level config now lives in the test sketch, not in a separate module config file.
static constexpr uint32_t SERIAL_BAUD = 115200;
static constexpr uint32_t UPDATE_INTERVAL_MS = 1000;
static constexpr uint32_t I2C_SPEED = 100000;

// Same I2C object style as MPU module
DepthSensor depthSensor(Wire);

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(500);

    Serial.println();
    Serial.println("=== Depth Sensor Module Test ===");

    // System owns the bus, not the module
    Wire.begin(21, 22);
    Wire.setClock(I2C_SPEED);

    if (!depthSensor.begin()) {
        Serial.println("Depth sensor init failed.");
        while (true) {
            delay(500);
        }
    }

    Serial.println("Depth sensor init OK.");
    Serial.println();

    Serial.println("Calibration:");
    Serial.println("  Zero Voltage : 0.8536 V");
    Serial.println("  Cal Voltage  : 0.9174 V @ 0.203 m");
    Serial.println("  Max Depth    : 5.0 m");
    Serial.println();
}

void loop() {
    depthSensor.update();

    Serial.println("-------------------");
    Serial.print("ADC: ");
    Serial.println(depthSensor.rawAdc());

    Serial.print("V: ");
    Serial.print(depthSensor.voltage(), 4);
    Serial.println(" V");

    Serial.print("Depth: ");
    Serial.print(depthSensor.depthMeters(), 3);
    Serial.println(" m");

    Serial.print("Depth: ");
    Serial.print(depthSensor.depthCentimeters(), 1);
    Serial.println(" cm");

    Serial.print("Status: ");
    Serial.println(DepthSensor::statusToString(depthSensor.status()));

    delay(UPDATE_INTERVAL_MS);
}