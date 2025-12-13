#include <SPI.h>
#include "ms5540c.h"

ms5540c sensor;

void setup() {
    Serial.begin(115200);
    SPI.begin(); // Sensor doesn't start SPI communication itself so we're to enable it ourselves
    sensor.begin();
}

void loop() {
    const float temp = -1*sensor.getTemperature();  // Temperature in degrees Celsius
    const float pressure = -1*sensor.getPressure() + 594; // pressure in mbar
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" C");
    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" mbar");
    Serial.print("Depth: ");
    Serial.print(((pressure-1013.30)/9810.0)*1000);
    Serial.println(" cm");
    // Serial.print("          ");
    // Serial.print(conv::mbarToAtm(pressure));
    // Serial.println(" atm");
    // Serial.print("          ");
    // Serial.println(conv::mbarToPascal(pressure));
    // Serial.println(" pas");
    Serial.println("\n=======================\n");

    delay(500);
}