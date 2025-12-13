#include "jsn.h"

JSNSensor jsn;

void setup() {
    Serial.begin(115200);
    JSN_init(&jsn, 4, 5, 30000);  // Trigger: GPIO4, Echo: GPIO5
}

void loop() {
    float dist = JSN_getDistance(&jsn);
    if (dist < 0) {
        Serial.println("❌ Out of range");
    } else {
        Serial.print("✅ Distance: ");
        Serial.print(dist);
        Serial.println(" cm");
    }
    delay(1000);
}
