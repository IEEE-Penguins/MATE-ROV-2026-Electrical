#include "PCA.h"

PCA pwm;

Motor thrusters[] = {{THRUSTER_TYPE, 0}, {THRUSTER_TYPE, 3}};

#define thruster_size sizeof(thrusters) / sizeof(thrusters[0])

void setup()
{
    Serial.begin(9600);
    pwm.beginPCA(thrusters, thruster_size);
    Serial.println("Test from setup");
}

void loop() {
  if (Serial.available() > 0) {
    // Read numeric input from Serial Monitor
    int usInput = Serial.parseInt();

    // Check for a valid ESC range (usually 1000us to 2000us)
    if (usInput >= 10 && usInput <= 5000) {
      Serial.print("Setting Pulse to: ");
      Serial.print(usInput);
      Serial.println(" us");

      // Use the library's built-in microsecond function
    for (byte i = 0; i < thruster_size; i++)
      pwm.pwmDrive(thrusters[i].type, usInput, thrusters[i].channel);
    } 
    else if (usInput != 0) {
      Serial.println("Invalid range! Please enter a value between 1000 and 2000.");
    }
  }
}
