#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN 102
#define SERVOMAX 800

String input = "";

void setup() {
  pwm.begin();
  pwm.setPWMFreq(50);
  Serial.begin(115200);
  Serial.println("🎯 تحكم في السيرفو. اكتب مثلاً: M0:45 أو M3:120");
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      processInput(input);
      input = "";
    } else {
      input += c;
    }
  }
}

void processInput(String cmd) {
  cmd.trim();
  cmd.toUpperCase();

  if (cmd.startsWith("M") && cmd.indexOf(":") > 1) {
    int channel = cmd.substring(1, cmd.indexOf(":")).toInt();
    int angle = cmd.substring(cmd.indexOf(":") + 1).toInt();

      int pulse = angle;
      pwm.setPWM(channel, 0, pulse);
      Serial.print(pwm.getPWM(0, true));
      Serial.print("--");
      Serial.println(pwm.getPWM(0, true));
    
  } else {
    Serial.println("❌ أمر غير صحيح. استخدم الصيغة: M0:زاوية");
  }
}

//> 360 blk -> 4
//> 180 blk -> 5
//> 180 ble -> 7
