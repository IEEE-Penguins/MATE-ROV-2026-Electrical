#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const int potPin = A0;       // Potentiometer analog input pin
const int servoPin = 3;     // Servo PWM output pin

Servo myServo;               // Create servo object

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool is_display = false;

void setup() {
  Serial.begin(115200);
  myServo.attach(servoPin);  // Attach servo to pin
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    is_display = true;
    display.clearDisplay();
    display.display();
  }
}

void loop() {
  int potValue = analogRead(potPin);              // Read analog value (0–4095)
  int angle = map(potValue, 100, 1023, 0, 180);      // Map to servo angle (0–180)
  myServo.write(angle);                           // Move servo
  Serial.print("Pot: ");
  Serial.print(potValue);
  Serial.print("  Angle: ");
  Serial.println(angle);

  if(is_display) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.print("angle: ");
    display.println(angle);
    display.display();
  }

  delay(15); // Small delay for stability
}
