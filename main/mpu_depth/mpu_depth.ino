#include <Wire.h>
#include "depth.h"
#include "MPU.h"   // Change to your actual MPU header name if needed

// =========================
// App-level test config
// =========================
static constexpr uint32_t SERIAL_BAUD      = 115200;
static constexpr uint32_t I2C_SPEED        = 100000;
static constexpr uint8_t  I2C_SDA_PIN      = 21;
static constexpr uint8_t  I2C_SCL_PIN      = 22;
static constexpr uint32_t UPDATE_INTERVAL  = 500;

// =========================
// Shared I2C bus devices
// =========================
DepthSensor depthSensor(Wire);
MPU6050 mpu(Wire);   // Adjust class name if your MPU class is named differently

// =========================
// Utility: I2C scan
// =========================
void scanI2CBus(TwoWire &bus) {
    Serial.println("Scanning I2C bus...");

    uint8_t found = 0;

    for (uint8_t addr = 1; addr < 127; ++addr) {
        bus.beginTransmission(addr);
        uint8_t error = bus.endTransmission();

        if (error == 0) {
            Serial.print("  Found device at 0x");
            if (addr < 16) Serial.print('0');
            Serial.println(addr, HEX);
            found++;
        }
    }

    if (found == 0) {
        Serial.println("  No I2C devices found.");
    } else {
        Serial.print("  Total devices found: ");
        Serial.println(found);
    }

    Serial.println();
}

// =========================
// Utility: print section line
// =========================
void printDivider() {
    Serial.println("--------------------------------------------------");
}

// =========================
// Setup
// =========================
void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(500);

    Serial.println();
    Serial.println("=== Shared I2C Bus Test: MPU + Depth ===");
    Serial.println();

    // One shared bus for both modules
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(I2C_SPEED);

    scanI2CBus(Wire);

    bool depthOk = depthSensor.begin();
    bool mpuOk   = mpu.begin();

    Serial.print("Depth init: ");
    Serial.println(depthOk ? "OK" : "FAILED");

    Serial.print("MPU init:   ");
    Serial.println(mpuOk ? "OK" : "FAILED");

    Serial.println();

    if (!depthOk || !mpuOk) {
        Serial.println("One or more devices failed to initialize.");
        Serial.println("Check addresses, power, pull-ups, and wiring.");
    } else {
        Serial.println("Both devices initialized on the same I2C bus.");
    }

    Serial.println();
}

// =========================
// Loop
// =========================
void loop() {
    bool depthReadOk = depthSensor.update();
    bool mpuReadOk   = mpu.update();

    printDivider();

    // ---- Depth ----
    Serial.println("[DEPTH]");
    Serial.print("update(): ");
    Serial.println(depthReadOk ? "OK" : "FAILED");

    Serial.print("rawAdc: ");
    Serial.println(depthSensor.rawAdc());

    Serial.print("voltage: ");
    Serial.print(depthSensor.voltage(), 4);
    Serial.println(" V");

    Serial.print("depth: ");
    Serial.print(depthSensor.depthMeters(), 3);
    Serial.println(" m");

    Serial.print("status: ");
    Serial.println(DepthSensor::statusToString(depthSensor.status()));

    Serial.println();

    // ---- MPU ----
    Serial.println("[MPU]");
    Serial.print("update(): ");
    Serial.println(mpuReadOk ? "OK" : "FAILED");

    // These getter names are the only part you may need to adapt
    // to your final MPU module API.
    Serial.print("accX: ");
    Serial.print(mpu.accX(), 3);
    Serial.print("  accY: ");
    Serial.print(mpu.accY(), 3);
    Serial.print("  accZ: ");
    Serial.println(mpu.accZ(), 3);

    Serial.print("gyroX: ");
    Serial.print(mpu.gyroX(), 3);
    Serial.print("  gyroY: ");
    Serial.print(mpu.gyroY(), 3);
    Serial.print("  gyroZ: ");
    Serial.println(mpu.gyroZ(), 3);

    Serial.print("roll: ");
    Serial.print(mpu.roll(), 2);
    Serial.print("  pitch: ");
    Serial.print(mpu.pitch(), 2);
    Serial.print("  yaw: ");
    Serial.println(mpu.yaw(), 2);

    Serial.println();

    delay(UPDATE_INTERVAL);
}