#include <Servo.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// ==========================================
// 1. تعريفات الهاردوير
// ==========================================
#define ENC_PIN 2         // Encoder pin

#define ALL_MOTORS_PWM 6
#define IN1 13
#define IN2 12
#define IN3 4
#define IN4 5

#define SERVO_PIN 11

// ==========================================
// 2. إعدادات النظام
// ==========================================
#define PID_INTERVAL 50        // 50ms فترة تحديث الـ PID
#define MIN_PWM 60             // أقل PWM لتحريك الموتور (Start torque)
#define MAX_PWM 255
#define STABILIZER_THRESHOLD .5
#define FILTER_ALPHA 0.965      // رجعنا الـ Alpha زي ما كانت للكود الأصلي

// ==========================================
// 3. متغيرات الـ PID (القيم الجديدة الهادئة)
// ==========================================
// القيم دي عشان تعالج التذبذب اللي ظهر في اللوج
double Kp = 0.15;      // كان 0.5 (قللناه عشان العصبية)
double Ki = 0.8;       // كان 7.5 (قللناه جداً عشان ده سبب المشكلة الرئيسي)
double Kd = 0.05;      // قيمة صغيرة للتنعيم

double Setpoint_RPM = 0;
double Input_RPM = 0;
double Output_PWM = 0;

double error = 0;
double lastError = 0;
double integral = 0;
double derivative = 0;

// ==========================================
// 4. متغيرات عامة
// ==========================================
Servo stabilizer;
Adafruit_MPU6050 mpu;

volatile long ticks = 0;
bool isRunning = false;
const int PPR = 20;

float filteredPitch = 0;
float lastServoPos = 90;

unsigned long lastLoopTime = 0;
unsigned long lastPIDTime = 0;
unsigned long lastLogTime = 0;

// متغيرات للـ Debug
int debugMode = 1; // 0=off, 1=basic, 2=detailed

void setup() {
Serial.begin(9600);

// إعداد المواتير
pinMode(ALL_MOTORS_PWM, OUTPUT);
pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);

analogWrite(ALL_MOTORS_PWM, 0);

// إعداد السيرفو (كما هو في كودك الأصلي)
stabilizer.attach(SERVO_PIN);
stabilizer.write(90);

// تشغيل الحساس
if (!mpu.begin()) {
Serial.println("❌ MPU6050 Failed!");
while(1) delay(100);
}
mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
mpu.setGyroRange(MPU6050_RANGE_500_DEG);
mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

// إعداد Encoder
pinMode(ENC_PIN, INPUT_PULLUP);
attachInterrupt(digitalPinToInterrupt(ENC_PIN), countTicks, RISING);

lastLoopTime = millis();
lastPIDTime = millis();
}

void loop() {
unsigned long currentTime = millis();

// ==========================================
// A. توازن المنصة (كودك الأصلي تماماً)
// ==========================================
sensors_event_t a, g, temp;
mpu.getEvent(&a, &g, &temp);

float dt_filter = (currentTime - lastLoopTime) / 1000.0;
lastLoopTime = currentTime;

float accelPitch = atan2(a.acceleration.x, sqrt(pow(a.acceleration.y, 2) + pow(a.acceleration.z, 2))) * 180 / PI;
float gyroRate = g.gyro.y * 180 / PI;

filteredPitch = FILTER_ALPHA * (filteredPitch + gyroRate * dt_filter) + (1 - FILTER_ALPHA) * accelPitch;

int targetPos = 90 + filteredPitch;
targetPos = constrain(targetPos, 0, 180);

if (abs(targetPos - lastServoPos) > STABILIZER_THRESHOLD) {
stabilizer.write(targetPos);
lastServoPos = targetPos;
}

// ==========================================
// B. مثبت السرعة PID (المعدل لعلاج التذبذب)
// ==========================================
if (currentTime - lastPIDTime >= PID_INTERVAL) {
double dt_pid = (currentTime - lastPIDTime) / 1000.0;

// حساب RPM من Encoder  
noInterrupts();  
long currentTicks = ticks;  
ticks = 0;  
interrupts();  
  
// حساب RPM اللحظي  
double raw_RPM = (currentTicks / (float)PPR) * (60.0 / dt_pid);  
  
// فلتر قوي لتنعيم القراءة (Exponential Moving Average)  
// ده بيمنع القفزات اللي بتجنن الـ PID  
Input_RPM = (Input_RPM * 0.8) + (raw_RPM * 0.2);  
  
lastPIDTime = currentTime;  

if (isRunning && Setpoint_RPM > 0) {  
  // 1. حساب الخطأ  
  error = Setpoint_RPM - Input_RPM;  
    
  // 2. التكامل (مع تقليل الـ Windup)  
  integral += error * dt_pid;  
  // قللنا الحد الأقصى للتكامل عشان ميركمش قيم غلط  
  integral = constrain(integral, -50, 50);   
    
  // 3. التفاضل  
  derivative = (error - lastError) / dt_pid;  
    
  // معادلة PID بالقيم الجديدة  
  double pidOutput = (Kp * error) + (Ki * integral) + (Kd * derivative);  
    
  // 4. Feed Forward (دفعة مبدئية)  
  double basePWM = map(Setpoint_RPM, 0, 1000, 50, 255);  
    
  Output_PWM = basePWM + pidOutput;  
    
  // ---------------------------------------------------------  
  // تعديل هام: السماح بالنزول للصفر عند الفرملة  
  // ---------------------------------------------------------  
  if (Output_PWM < MIN_PWM && Output_PWM > 10) {  
     Output_PWM = MIN_PWM; // لو قيمة صغيرة بس موجبة، خليها Min عشان الموتور يزق  
  } else if (Output_PWM <= 10) {  
     Output_PWM = 0;       // اسمح له يقف لو محتاج يفرمل  
  }  
    
  Output_PWM = constrain(Output_PWM, 0, 255);  
    
  lastError = error;  
    
  analogWrite(ALL_MOTORS_PWM, (int)Output_PWM);  
    
} else {  
  analogWrite(ALL_MOTORS_PWM, 0);  
  // تصفير مهم جداً عشان لما تبدأ تاني ميبدأش مجنون  
  integral = 0;  
  lastError = 0;  
  Input_RPM = 0;  
  Output_PWM = 0;  
  error = 0;  
}

}

// ==========================================
// C. استقبال البلوتوث
// ==========================================
if (Serial.available() > 0) {
String input = Serial.readStringUntil('\n');
input.trim();
input.toUpperCase();

if (input.equals("O")) {  
  isRunning = true;  
  integral = 0;   
  lastError = 0;  
  Serial.println("✅ Motor ON");  
}  
else if (input.equals("F")) {  
  isRunning = false;  
  Setpoint_RPM = 0;  
  Serial.println("⛔ Motor OFF");  
}  
else if (input.startsWith("P")) {  
  Kp = input.substring(1).toFloat();  
  Serial.print("Kp = "); Serial.println(Kp, 2);  
}  
else if (input.startsWith("I")) {  
  Ki = input.substring(1).toFloat();  
  integral = 0;   
  Serial.print("Ki = "); Serial.println(Ki, 2);  
}  
else if (input.startsWith("D")) {  
  Kd = input.substring(1).toFloat();  
  Serial.print("Kd = "); Serial.println(Kd, 2);  
}  
else {  
  int val = input.toInt();  
  if (val >= 0 && val <= 1000) {  
    Setpoint_RPM = val;   
    Serial.print("🎯 Target RPM: "); Serial.println(Setpoint_RPM);  
  }  
}

}

// ==========================================
// D. الطباعة للمراقبة
// ==========================================
if (currentTime - lastLogTime >= 300) {
lastLogTime = currentTime;

if (debugMode == 1) {  
  Serial.print("RPM:");  
  Serial.print((int)Input_RPM);  
  Serial.print(" / ");  
  Serial.print((int)Setpoint_RPM);  
  Serial.print(" | PWM:");  
  Serial.print((int)Output_PWM);  
  Serial.print(" | Err:");  
  Serial.println((int)error);  
}  
else if (debugMode == 2) {  
  Serial.print("Target:"); Serial.print((int)Setpoint_RPM);  
  Serial.print(" | Current:"); Serial.print(Input_RPM, 1);  
  Serial.print(" | PWM:"); Serial.print((int)Output_PWM);  
  Serial.print(" | Err:"); Serial.print(error, 1);  
  Serial.print(" | I:"); Serial.print(integral, 2);  
  Serial.print(" | D:"); Serial.println(derivative, 2);  
}

}
}

// ==========================================
// Interrupt للـ Encoder
// ==========================================
void countTicks() {
ticks++;
}
