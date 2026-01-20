#include "depth.h"

DepthSensor::DepthSensor() {
    mA_per_meter = 0;
}

bool DepthSensor::begin() {
    // Init I2C
    Wire.begin(SDA_PIN, SCL_PIN);
    
    // Init ADS1115
    if (!ads.begin(0x48)) {
        Serial.println("ADS1115 ERROR!");
        Serial.println("Check wiring:");
        Serial.println("VDD->3.3V, GND->GND");
        Serial.println("SDA->GPIO21, SCL->GPIO22");
        Serial.println("ADDR->GND");
        return false;
    }
    
    Serial.println("ADS1115 OK");
    ads.setGain(GAIN_ONE);
    
    // Calculate cal factor
    mA_per_meter = (I_CAL - I_ZERO) / DEPTH_CAL;
    
    return true;
}

void DepthSensor::printCalibration() {
    Serial.println("Calibration:");
    Serial.print("  Zero: "); Serial.print(I_ZERO); Serial.println(" mA");
    Serial.print("  Cal: "); Serial.print(I_CAL); Serial.print(" mA @ ");
    Serial.print(DEPTH_CAL * 100); Serial.println(" cm");
    Serial.print("  Factor: "); Serial.print(mA_per_meter, 3); 
    Serial.println(" mA/m\n");
    
    Serial.print("Max Range: 0-");
    Serial.print(MAX_DEPTH);
    Serial.println("m\n");
}

void DepthSensor::readSensor() {
    // Read ADC
    int16_t adc = ads.readADC_SingleEnded(0);
    
    // Calculate voltage
    float voltage = adc * 0.000125;
    
    // Calculate current
    float current = (voltage / SHUNT_R) * 1000.0;
    
    // Calculate depth
    float depth = calcDepth(current);
    
    //Results
    Serial.println("-------------------");
    Serial.print("ADC: "); Serial.println(adc);
    Serial.print("V: "); Serial.print(voltage, 3); Serial.println(" V");
    Serial.print("I: "); Serial.print(current, 2); Serial.println(" mA");
    Serial.print("Depth: "); Serial.print(depth, 2); Serial.println(" m");
    Serial.print("Depth: "); Serial.print(depth * 100, 1); Serial.println(" cm");
    
    // Calculate percentage
    float percentage = (depth / MAX_DEPTH) * 100;
    Serial.print("Level: "); Serial.print(percentage, 1); Serial.println(" %");
    
    //status
    checkStatus(current, voltage);
    Serial.println();
}

float DepthSensor::calcDepth(float current) {
    // If current is at or below zero point, depth is 0
    if (current <= I_ZERO) {
        return 0;
    }
    
    // Calculate depth based on calibration
    float depth = (current - I_ZERO) / mA_per_meter;
    
    // Constrain to valid range
    if (depth < 0) depth = 0;
    if (depth > MAX_DEPTH) depth = MAX_DEPTH;
    
    return depth;
}

void DepthSensor::checkStatus(float current, float voltage) {
    if (voltage < 0.5) {
        Serial.println("ERROR: Disconnected");
    }
    else if (current < I_ZERO - 1.0) {
        Serial.println("WARN: Low current");
    }
    else if (current > 20.0) {
        Serial.println("WARN: High current");
    }
    else {
        Serial.println("OK");
    }
}