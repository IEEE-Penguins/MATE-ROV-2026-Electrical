#include <Arduino.h>
#include "t100.h"

ESCController esc;

void setup() {
  Serial.begin(115200);
  esc.init();
}

void loop() {
  // Test motors 0 to 7 with increasing values
  for (int i = 0; i < 8; i++) {
    Serial.print("Testing motor ");
    Serial.println(i);
    esc.set(i, 150); // Forward
    delay(1000);
    esc.set(i, 150, false); // Reverse
    delay(1000);
    esc.set(i, 0); // Stop
    delay(1000);
  }

  while (true); // Stop after one round
}
