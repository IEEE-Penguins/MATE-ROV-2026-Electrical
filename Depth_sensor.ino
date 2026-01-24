#include <Wire.h>

// Config
#define SHUNT_R 220.0
#define MAX_DEPTH 5.0

#define I_ZERO 4.84
#define I_CAL 5.22
#define DEPTH_CAL 0.22

float mA_per_meter;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("\n=== Depth Sensor (Calibrated) ==="));
  
  Wire.begin();
  
  mA_per_meter = (I_CAL - I_ZERO) / DEPTH_CAL;
  
  Serial.println(F("Calibration:"));
  Serial.print(F("  Zero: ")); Serial.print(I_ZERO); Serial.println(F(" mA"));
  Serial.print(F("  Cal: ")); Serial.print(I_CAL); Serial.print(F(" mA @ "));
  Serial.print(DEPTH_CAL * 100); Serial.println(F(" cm"));
  Serial.print(F("  Factor: ")); Serial.print(mA_per_meter, 3); 
  Serial.println(F(" mA/m\n"));
  
  Serial.print(F("Max Range: 0-"));
  Serial.print(MAX_DEPTH);
  Serial.println(F("m\n"));
  
  delay(500);
}

void loop() {
  int16_t adc = analogRead(A0);
  float v = (adc / 1023.0) * 5.0;
  float i = (v / SHUNT_R) * 1000.0;
  float d = calcDepth(i);
  
  Serial.println(F("-------------------"));
  Serial.print(F("ADC: ")); Serial.println(adc);
  Serial.print(F("V: ")); Serial.print(v, 3); Serial.println(F(" V"));
  Serial.print(F("I: ")); Serial.print(i, 2); Serial.println(F(" mA"));
  Serial.print(F("Depth: ")); Serial.print(d, 2); Serial.println(F(" m"));
  Serial.print(F("Depth: ")); Serial.print(d * 100, 1); Serial.println(F(" cm"));
  
  float pct = (d / MAX_DEPTH) * 100;
  Serial.print(F("Level: ")); Serial.print(pct, 1); Serial.println(F(" %"));
  
  checkStatus(i, v);
  Serial.println();
  
  delay(1000);
}

float calcDepth(float i) {
  if (i <= I_ZERO) {
    return 0;
  }
  float d = (i - I_ZERO) / mA_per_meter;
  
  if (d < 0) d = 0;
  if (d > MAX_DEPTH) d = MAX_DEPTH;
  
  return d;
}

void checkStatus(float i, float v) {
  if (v < 0.5) {
    Serial.println(F("ERROR: Disconnected"));
  }
  else if (i < I_ZERO - 1.0) {
    Serial.println(F("WARN: Low current"));
  }
  else if (i > 20.0) {
    Serial.println(F("WARN: High current"));
  }
  else {
    Serial.println(F("OK"));
  }
}