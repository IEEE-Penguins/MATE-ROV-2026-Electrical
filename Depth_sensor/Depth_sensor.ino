#include "depth.h"        
#include "depthConfig.h"  
// Create sensor object
DepthSensor sensor;

void setup() {
    // Init Serial
    Serial.begin(SERIAL_BAUD);
    delay(500);
    
    Serial.println("\n=== Depth Sensor (Calibrated) - ESP32 ===");
    
    #if ENABLE_STATUS_LED
        pinMode(STATUS_LED_PIN, OUTPUT);
        digitalWrite(STATUS_LED_PIN, LOW);
    #endif
    
    // Init sensor
    if (!sensor.begin()) {
        Serial.println("Failed to initialize sensor!");
        while (1) {
            #if ENABLE_STATUS_LED
                digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
            #endif
            delay(500);
        }
    }
    
    #if ENABLE_STATUS_LED
        digitalWrite(STATUS_LED_PIN, HIGH);
    #endif
    
    // Print cali info
    sensor.printCalibration();
    
    delay(500);
}

void loop() {
    // Read and display sensor data
    sensor.readSensor();
    delay(UPDATE_INTERVAL);
}