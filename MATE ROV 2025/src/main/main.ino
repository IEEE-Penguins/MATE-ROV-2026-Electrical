
//> libraries
#include <Arduino.h>
#include "Ultrasonic.h"
#include "PH.h"
#include "MPU.h"
// #include "config.h"

//> pins definition
#define rx_trig0 PB14
#define tx_echo0 PB13
#define rx_trig1 PB0
#define tx_echo1 PB1
#define ph_sensor PA0
#define mpu_SCL PB6
#define mpu_SDA PB7
#define light_pin PB9




//> ph object
DFRobot_PH ph_object;

//> mpu object
Adafruit_MPU6050 mpu_object;

//> global temperature value
float temperature = 25;

//> light state
int light_state = 0;


void setup() {
  
  //> serial setup
  Serial.begin(9600); // USB Serial
  Serial2.begin(9600); // UART2 (PA2 TX, PA3 RX)

  //> pins setup
  pinMode(rx_trig0, OUTPUT);
  pinMode(tx_echo0, INPUT);
  pinMode(rx_trig1, OUTPUT);
  pinMode(tx_echo1, INPUT);
  pinMode(ph_sensor, INPUT);
  pinMode(light_pin, OUTPUT);

  //> setup ph
  ph_object.begin();

  //> mpu setup
  setup_MPU(mpu_SCL, mpu_SDA, mpu_object);

}

void loop() {

  //> read the data from the USB serial and send it to UART2 serial
  if(Serial.available()) {
    String data = Serial.readStringUntil('\n'); 
    //> test logic
    // if(data[0] == '1') light_state = 1;
    // else if(data[0] == '0') light_state = 0;
    Serial2.print(data);
  }

  // > calculate the distance readings of the two ultrasonic sensors
  float dis1 = get_distance(rx_trig0, tx_echo0, 344.39);
  float dis2 = get_distance(rx_trig1, tx_echo1, 344.39);

  //> get mpu readings from the sensor
  MPU_Readings mpu_readings = get_MPU_readings(mpu_object);

  //> update the temperature
  temperature = mpu_readings.temp;

  //> update light state
  digitalWrite(light_pin, light_state);

  //> read ph value
  float ph_value = get_ph(ph_sensor, temperature, ph_object);

  //> print the values to the USB serial
  //> format: dis1,dis2,ph,temp,acc_x,acc_y,acc_z,gyro_x,gyro_y,gyro_z
  Serial.print(dis1);
  Serial.print(",");
  Serial.print(dis2);
  Serial.print(",");
  Serial.print(ph_value);
  Serial.print(",");
  Serial.print(mpu_readings.temp);
  Serial.print(",");
  Serial.print(mpu_readings.acc_x);
  Serial.print(",");
  Serial.print(mpu_readings.acc_y);
  Serial.print(",");
  Serial.print(mpu_readings.acc_z);
  Serial.print(",");
  Serial.print(mpu_readings.gyro_x);
  Serial.print(",");
  Serial.print(mpu_readings.gyro_y);
  Serial.print(",");
  Serial.print(mpu_readings.gyro_z);
  Serial.println();

  delay(50);
}
