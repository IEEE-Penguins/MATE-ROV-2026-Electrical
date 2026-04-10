// #define DIR_PIN 26
// #define STEP_PIN 27

// float stepsPerRevolution = 200.0;
// float stepAngle = 360.0 / stepsPerRevolution;

// int currentPosition = 0; // بالـ steps

// void moveToAngle(float angle) {
//   int targetSteps = angle / stepAngle;
//   int stepsToMove = targetSteps - currentPosition;

//   if (stepsToMove > 0) {
//     digitalWrite(DIR_PIN, HIGH);
//   } else {
//     digitalWrite(DIR_PIN, LOW);
//     stepsToMove = -stepsToMove;
//   }

//   for (int i = 0; i < stepsToMove; i++) {
//     digitalWrite(STEP_PIN, HIGH);
//     delayMicroseconds(2000);
//     digitalWrite(STEP_PIN, LOW);
//     delayMicroseconds(2000);
//   }

//   currentPosition = targetSteps;
// }

// void setup() {
//   pinMode(STEP_PIN, OUTPUT);
//   pinMode(DIR_PIN, OUTPUT);
//   Serial.begin(115200);

//   Serial.println("Enter angle:");
// }

// void loop() {
//   if (Serial.available()) {
//     float angle = Serial.parseFloat(); // اقرأ الزاوية

//     Serial.print("Moving to: ");
//     Serial.println(angle);

//     moveToAngle(angle);
//   }
// }





// #define DIR_PIN     25
// #define STEP_PIN    26
// #define ENABLE_PIN  27

// int stepDelay = 1200;

// void setup() {
//   pinMode(DIR_PIN, OUTPUT);
//   pinMode(STEP_PIN, OUTPUT);
//   pinMode(ENABLE_PIN, OUTPUT);

//   digitalWrite(ENABLE_PIN, LOW); // LOW → الموتور شغال
//   digitalWrite(DIR_PIN, HIGH);   // اتجاه ثابت
// }

// void loop() {
//   digitalWrite(STEP_PIN, HIGH);
//   delayMicroseconds(stepDelay);
//   digitalWrite(STEP_PIN, LOW);
//   delayMicroseconds(stepDelay);
// }





// #define DIR_PIN     26
// #define STEP_PIN    27

// int stepDelay = 2000; 

// void setup() {
//   pinMode(DIR_PIN, OUTPUT);
//   pinMode(STEP_PIN, OUTPUT);

//   digitalWrite(DIR_PIN, HIGH);   // اتجاه ثابت
// }

// void loop() {
//   digitalWrite(STEP_PIN, HIGH);
//   delayMicroseconds(stepDelay);
//   digitalWrite(STEP_PIN, LOW);
//   delayMicroseconds(stepDelay);
// }





// // ESP32 + DRV8825 Stepper Motor - دوران في اتجاه واحد

// #define DIR_PIN     25
// #define STEP_PIN    26
// #define ENABLE_PIN  27

// int stepsPerRevolution = 200;  // عدد الخطوات في دورة كاملة
// int stepDelay = 1200;          // ميكروثانية بين كل خطوة

// void setup() {
//   pinMode(DIR_PIN, OUTPUT);
//   pinMode(STEP_PIN, OUTPUT);
//   pinMode(ENABLE_PIN, OUTPUT);

//   digitalWrite(ENABLE_PIN, LOW); // LOW → الموتور شغال
//   digitalWrite(DIR_PIN, HIGH);   // اتجاه ثابت
// }

// void loop() {
//   // لف بشكل مستمر في نفس الاتجاه
//   for(int i = 0; i < stepsPerRevolution; i++){
//     digitalWrite(STEP_PIN, HIGH);
//     delayMicroseconds(stepDelay);
//     digitalWrite(STEP_PIN, LOW);
//     delayMicroseconds(stepDelay);
//   }

//   delay(500);
// }





// // ESP32 + DRV8825 Stepper Motor Example

// #define DIR_PIN     25
// #define STEP_PIN    26
// #define ENABLE_PIN  27

// int stepsPerRevolution = 200;
// int stepDelay = 1200;          // ميكروثانية بين كل خطوة (لو الموتور بيزن زود الرقم)

// void setup() {
//   pinMode(DIR_PIN, OUTPUT);
//   pinMode(STEP_PIN, OUTPUT);
//   pinMode(ENABLE_PIN, OUTPUT);

//   digitalWrite(ENABLE_PIN, LOW); // LOW → الموتور شغال
//   digitalWrite(DIR_PIN, HIGH);   // تحديد الاتجاه
// }

// void loop() {
//   // لف دورة كاملة
//   for(int i = 0; i < stepsPerRevolution; i++){
//     digitalWrite(STEP_PIN, HIGH);
//     delayMicroseconds(stepDelay);
//     digitalWrite(STEP_PIN, LOW);
//     delayMicroseconds(stepDelay);
//   }

//   delay(1000); // وقفة قبل قلب الاتجاه

//   // قلب الاتجاه
//   digitalWrite(DIR_PIN, !digitalRead(DIR_PIN));
// }