#ifndef CONFIG_H
#define CONFIG_H


#define SERIAL_BAUD 115200

#define UPDATE_INTERVAL 1000 

#define I2C_SPEED 100000     

#define ADS1115_ADDR 0x48

#define ENABLE_DEBUG true
#define ENABLE_STATUS_LED false

#if ENABLE_STATUS_LED
#define STATUS_LED_PIN 2
#endif

#endif