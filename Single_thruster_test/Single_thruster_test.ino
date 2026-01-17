#include "ESC.h"

#define motor_bin 15

ESCChannel Motor(motor_bin, ESC_STOP);

void setup() {
  Serial.begin(115200);
}

void loop() {
  Motor.drive(1300);
  delay(3000);
  Motor.stop();
  delay(3000);
  Motor.drive(1700);
  delay(3000);
  Motor.stop();
  delay(3000);
}

// // Pins
// const int pwmPin = 15;  // Output
// const int readPin = 12; // Input

// // PWM Settings
// const int freq = 5000;
// const int resolution = 8; // 0-255 range

// #include "ESC.h"
// ESCChannel Motor(15, ESC_STOP);

// void setup() {
//   Serial.begin(115200);

//   // NEW API: This replaces ledcSetup and ledcAttachPin
//   // Syntax: ledcAttach(pin, frequency, resolution)
//   ledcAttach(pwmPin, freq, resolution);

//   // Configure Input Pin
//   pinMode(readPin, INPUT);
// }

// void loop() {
//   // Set duty cycle to 50% (127 out of 255)
//   //ledcWrite(pwmPin, 127); // Note: In v3.0, ledcWrite uses the PIN, not a channel
//   Motor.drive(1300);
//   // Read the pulse width
//   unsigned long highTime = pulseIn(readPin, HIGH);
//   unsigned long lowTime = pulseIn(readPin, LOW);
//   unsigned long cycleTime = highTime + lowTime;

//   if (cycleTime > 0) {
//     float dutyCycle = (highTime / (float)cycleTime) * 100;
//     Serial.print("Measured Duty Cycle: ");
//     Serial.print(dutyCycle);
//     Serial.println("%");
//   }

//   delay(1000);
// }