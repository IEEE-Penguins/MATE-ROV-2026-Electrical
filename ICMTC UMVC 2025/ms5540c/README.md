# MS5540C Arduino Library

## Brief sensor description

Given sensor operates using slightly modified SPI interface (the main difference is that sensor does
not have SS/CS (Slave Select/Chip Select) line. Instead, this sensor receives 3 START and 3 STOP
bits before and after each bit control sequence. It can operate up to 100m below water surface (as
said in datasheet).

## Code example

Here's a simple example which measures current temperature and pressure in mbars each 3 seconds. For
further explanations on the comments read section "Explanations" below.

*Also, if you don't know AVR MCU's like the back of your hand, please carefully read "Important
notes" section to avoid different tricky problems.*

```ino
#include <SPI.h>
#include <ms5540c.h>

ms5540c sensor;

void setup() {
    Serial.begin(9600);
    SPI.begin(); // Sensor doesn't start SPI communication itself so we're to enable it ourselves
    sensor.begin();
}

void loop() {
    const float temp = sensor.getTemperature();  // Temperature in degrees Celsius
    const float pressure = sensor.getPressure(); // pressure in mbar
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" C");
    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" mbar");
    Serial.print("          ");
    Serial.print(conv::mbarToAtm(pressure));
    Serial.println(" atm");
    Serial.print("          ");
    Serial.println(conv::mbarToPascal(pressure));
    Serial.println(" pas");
    Serial.println("\n=======================\n");

    delay(3000);
}
```

## Important notes

### Connection with MCU features

Library uses hardware implementation of SPI library, so refer to actual pinout diagram of your MCU
to make sure which actual pins are reliable for MISO/MOSI/SCK lines.

**Very important note**: sensor lacks internal clock source so it is needed to provide external
clock source. It is made via MCLK pin. Make sure you've connected this input to `OC1A` pin (refer to
your actual MCU and Arduino pinout).

*Handy tip: if you're using Arduino Uno R3, `OC1A` is attached to pin D9.*

Connecting this input to some another pin will lead to sensor malfunctioning and you'll get strange
and incorrect measurement results (like negative pressure, etc.).

Note: it's better to have pullup resistors on serial lines to get rid of noise.
