#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

#define pot A0
#define btn 2
#define red_led 6
#define green_led 7
#define pwm_out 3
#define pwm_in 4

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Servo esc;

const int STOP_PWM = 1500;
const int MIN_PWM = 1100;
const int MAX_PWM = 1900;

enum Mode { FORWARD, REVERSE };
Mode currentMode = FORWARD;

bool lastButtonState = HIGH;

void setup() {bool is_display = false;
  pinMode(btn, INPUT_PULLUP);
  pinMode(green_led, OUTPUT);
  pinMode(red_led, OUTPUT);
  pinMode(pwm_in, INPUT);

  Serial.begin(115200);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
  

  esc.attach(pwm_out);

  esc.writeMicroseconds(1100);
  delay(3000);

}

void loop() {

  int speedPWM = STOP_PWM;
  int potValue = analogRead(pot);
  bool reading = digitalRead(btn);
  
  if (reading == LOW && lastButtonState == HIGH) {
    if (digitalRead(btn) == LOW) {
      esc.writeMicroseconds(1500);
      delay(300);
      currentMode = (currentMode == FORWARD) ? REVERSE : FORWARD;
    }
  }
  lastButtonState = reading;


  if (currentMode == FORWARD) {
    speedPWM = map(potValue, 0, 1023, STOP_PWM, MAX_PWM);
    digitalWrite(green_led, 1);
    digitalWrite(red_led, 0);
  } else {
    speedPWM = map(potValue, 0, 1023, STOP_PWM, MIN_PWM);
    digitalWrite(green_led, 0);
    digitalWrite(red_led, 1);
  }

  esc.writeMicroseconds(speedPWM);

  Serial.print("Mode: ");
  Serial.print(currentMode == FORWARD ? "FWD" : "REV");
  Serial.print(", PWM: ");
  Serial.print(speedPWM);
  Serial.println("μs");

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.print("Mode: ");
  display.println(currentMode == FORWARD ? "FWD" : "REV");
  display.setCursor(0, 35);
  display.print("PWM:");
  display.setCursor(70, 35);
  display.print(speedPWM);
  display.display();


  ////> testing
  // unsigned long pulseWidth = pulseIn(pwm_in, HIGH);
  // Serial.print("PWM Signal: ");
  // Serial.print(pulseWidth);
  // Serial.println(" microseconds");

}
