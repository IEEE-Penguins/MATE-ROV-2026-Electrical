#include <Arduino.h>
#include <cstring>
#include <cstdlib>
#include <HardwareSerial.h>

struct VALUES {
  int L2;
  int R2;
  int FORWARD;
  int BACKWARD;
  int LEFT;
  int RIGHT;
  int UP;
  int DOWN;
  int ROTATE_LEFT;
  int ROTATE_RIGHT;
};


HardwareSerial Serial1(USART1);

VALUES parseString(String input);

// Functions prototypes
void move_forward(int speed);
void move_backward(int speed);
void move_left(int speed);
void move_right(int speed);
void move_up(int speed);
void move_down(int speed);
void move_rotate_right(int speed);
void move_rotate_left(int speed);
void stop_motors();
void motor1(String dir);
void motor2(String dir);
void motor3(String dir);
void motor4(String dir);
void motor5(String dir);
void motor6(String dir);
void motor7(String dir);
void motor8(String dir);

#define IN1 PA0
#define IN2 PA1
#define IN3 PA2
#define IN4 PA3
#define IN5 PA4
#define IN6 PA5
#define IN7 PA6
#define IN8 PA7
#define IN9 PB0
#define IN10 PB1
#define IN11 PB2
#define IN12 PB10
#define IN13 PB12
#define IN14 PB13
#define IN15 PB14
#define IN16 PB15

#define EN1 PB9
#define EN2 PB8
#define EN3 PB7
#define EN4 PB6
#define EN5 PB5
#define EN6 PB4
#define EN7 PB3
#define EN8 PA15



const int directionPins[] = {
  IN1, IN2, IN3, IN4,
  IN5, IN6, IN7, IN8,
  IN9, IN10, IN11, IN12,
  IN13, IN14, IN15, IN16 
};

const int enablePins[] = {
  EN1, EN2, EN3, EN4,
  EN5, EN6, EN7, EN8 
};

void setup() {
  Serial.begin(115200);   //-> USB Serial for debugging
  Serial1.begin(115200);  //-> Enable USART1 (PA9 TX, PA10 RX)

  //-> Setup all direction pins
  for (int i = 0; i < 16; i++) {
    pinMode(directionPins[i], OUTPUT);
    digitalWrite(directionPins[i], LOW);
  }

  //-> Setup all enable pins
  for (int i = 0; i < 8; i++) {
    pinMode(enablePins[i], OUTPUT);
    analogWrite(enablePins[i], 0);
  }
}

void loop() {
  if (Serial1.available()) {
    // Read the incoming data as a full line
    String msg = Serial1.readStringUntil('\n');
    

  // int L2 = 0;
  // int R2 = 0;
  // int FORWARD = 0;
  // int BACKWARD = 0;
  // int LEFT = 0;
  // int RIGHT = 0;
  // int UP = 0;
  // int DOWN = 0;
  // int ROTATE_LEFT = 0;
  // int ROTATE_RIGHT = 0;

    // Parse the data
    VALUES data = parseString(msg);

    //test
    Serial.println(
      "L2: " + String(data.L2) + 
      ", R2: " + String(data.R2) + 
      ", FORWARD: " + String(data.FORWARD) + 
      ", BACKWARD: " + String(data.BACKWARD) + 
      ", LEFT: " + String(data.LEFT) + 
      ", RIGHT: " + String(data.RIGHT) + 
      ", ROTATE_LEFT: " + String(data.ROTATE_LEFT) + 
      ", ROTATE_RIGHT: " + String(data.ROTATE_RIGHT)
    );

    
    if (data.L2 > 0) move_up(data.L2);
    else if (data.R2 > 0) move_down(data.R2);
    else if (data.FORWARD > 0) move_forward(data.FORWARD);
    else if (data.BACKWARD > 0) move_backward(data.BACKWARD);
    else if (data.LEFT > 0) move_left(data.LEFT);
    else if (data.RIGHT > 0) move_right(data.RIGHT);
    else if (data.ROTATE_LEFT > 0) move_rotate_left(data.ROTATE_LEFT);
    else if (data.ROTATE_RIGHT > 0) move_rotate_right(data.ROTATE_RIGHT);
    else stop_motors();
  }
}

VALUES parseString(String input) {
  int values[10];  
  char buffer[50];  

  int L2 = 0;
  int R2 = 0;
  int FORWARD = 0;
  int BACKWARD = 0;
  int LEFT = 0;
  int RIGHT = 0;
  int UP = 0;
  int DOWN = 0;
  int ROTATE_LEFT = 0;
  int ROTATE_RIGHT = 0;

  input.toCharArray(buffer, sizeof(buffer));  

  char* token = strtok(buffer, ",");
  int i = 0;

  // Parse values from the string
  while (token != NULL && i < 10) {
    values[i++] = atoi(token);
    token = strtok(NULL, ",");
  }

  VALUES v;
  v.L2 = values[0];
  v.R2 = values[1];
  v.FORWARD = values[2];
  v.BACKWARD = values[3];
  v.LEFT = values[4];
  v.RIGHT = values[5];
  v.UP = values[6];
  v.DOWN = values[7];
  v.ROTATE_LEFT = values[8];
  v.ROTATE_RIGHT = values[9];

  return v;  


  

}

// this logic supposes that clock wise direction with the pump nozzle, and vice versa with counter clock wise.
void motor1(String dir){
  if(dir == "clock"){
    // Serial.println("Motor 1 Moving UP");
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  }
  else if(dir == "counter clock"){
    // Serial.println("Motor 1 Moving Down");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }
  else(){
    // Serial.println("Motor 1 Stops");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }
}

void motor2(String dir){
  if(dir == "clock"){
    // Serial.println("Motor 2 Moving UP");
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }
  else if(dir == "counter clock"){
    // Serial.println("Motor 2 Moving Down");
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }
  else(){
    // Serial.println("Motor 2 Stops");
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }
}

void motor3(String dir){
  if(dir == "clock"){
    // Serial.println("Motor 3 Moving UP");
    digitalWrite(IN5, HIGH);
    digitalWrite(IN6, LOW);
  }
  else if(dir == "counter clock"){
    // Serial.println("Motor 3 Moving Down");
    digitalWrite(IN5, LOW);
    digitalWrite(IN6, HIGH);
  }
  else(){
    // Serial.println("Motor 3 Stops");
    digitalWrite(IN5, LOW);
    digitalWrite(IN6, LOW);
  }
}

void motor4(String dir){
  if(dir == "clock"){
    // Serial.println("Motor 4 Moving UP");
    digitalWrite(IN7, HIGH);
    digitalWrite(IN8, LOW);
  }
  else if(dir == "counter clock"){
    // Serial.println("Motor 4 Moving Down");
    digitalWrite(IN7, LOW);
    digitalWrite(IN8, HIGH);
  }
  else(){
    // Serial.println("Motor 4 Stops");
    digitalWrite(IN7, LOW);
    digitalWrite(IN8, LOW);
  }
}

void motor5(String dir){
  if(dir == "clock"){
    // Serial.println("Motor 5 Moving UP");
    digitalWrite(IN9, HIGH);
    digitalWrite(IN10, LOW);
  }
  else if(dir == "counter clock"){
    // Serial.println("Motor 5 Moving Down");
    digitalWrite(IN9, LOW);
    digitalWrite(IN10, HIGH);
  }
  else(){
    // Serial.println("Motor 5 Stops");
    digitalWrite(IN9, LOW);
    digitalWrite(IN10, LOW);
  }
}

void motor6(String dir){
  if(dir == "clock"){
    // Serial.println("Motor 6 Moving UP");
    digitalWrite(IN11, HIGH);
    digitalWrite(IN12, LOW);
  }
  else if(dir == "counter clock"){
    // Serial.println("Motor 6 Moving Down");
    digitalWrite(IN11, LOW);
    digitalWrite(IN12, HIGH);
  }
  else(){
    // Serial.println("Motor 6 Stops");
    digitalWrite(IN11, LOW);
    digitalWrite(IN12, LOW);
  }
}

void motor7(String dir){
  if(dir == "clock"){
    // Serial.println("Motor 7 Moving UP");
    digitalWrite(IN13, HIGH);
    digitalWrite(IN14, LOW);
  }
  else if(dir == "counter clock"){
    // Serial.println("Motor 7 Moving Down");
    digitalWrite(IN13, LOW);
    digitalWrite(IN14, HIGH);
  }
  else(){
    // Serial.println("Motor 7 Stops");
    digitalWrite(IN13, LOW);
    digitalWrite(IN14, LOW);
  }
}

void motor8(String dir){
  if(dir == "clock"){
    // Serial.println("Motor 8 Moving UP");
    digitalWrite(IN15, HIGH);
    digitalWrite(IN16, LOW);
  }
  else if(dir == "counter clock"){
    // Serial.println("Motor 8 Moving Down");
    digitalWrite(IN15, LOW);
    digitalWrite(IN16, HIGH);
  }
  else(){
    // Serial.println("Motor 8 Stops");
    digitalWrite(IN15, LOW);
    digitalWrite(IN16, LOW);
  }
}

// Stop all motors /////////////////////////
void stop_motors() {
  Serial.println("STOP MOTORS");
  for (int i = 0; i < 16; i++) {
    digitalWrite(directionPins[i], LOW);
  }
  for (int i = 0; i < 8; i++) {
    analogWrite(enablePins[i], 0);
  }
}

void move_up(int speed) {
  Serial.println("Moving UP");
  motor1("clock");
  motor2("clock");
  analogWrite(EN1, speed);
  analogWrite(EN2, speed);
}

void move_down(int speed) {
  Serial.println("Moving DOWN");
  motor1("counter clock");
  motor2("counter clock");
  analogWrite(EN1, speed);
  analogWrite(EN2, speed);
}

void move_forward(int speed) {
  Serial.println("Moving FORWARD");
  motor3("clock");
  motor4("clock");
  motor5("clock");
  motor6("clock");
  analogWrite(EN3, speed);
  analogWrite(EN4, speed);
}

void move_backward(int speed) {
  Serial.println("Moving BACKWARD");
  motor3("counter clock");
  motor4("counter clock");
  motor5("counter clock");
  motor6("counter clock");
  analogWrite(EN3, speed);
  analogWrite(EN4, speed);
}

void move_left(int speed) {
  Serial.println("Moving LEFT");
  motor3("clock");
  motor5("clock");
  motor4("counter clock");
  motor6("counter clock");
  analogWrite(EN5, speed);
  analogWrite(EN6, speed);
}

void move_right(int speed) {
  Serial.println("Moving RIGHT");
  motor3("counter clock");
  motor5("counter clock");
  motor4("clock");
  motor6("clock");
  analogWrite(EN5, speed);
  analogWrite(EN6, speed);
}

void move_rotate_left(int speed) { // still not finished yet
  Serial.println("Rotating LEFT");
  motor7("clock");
  motor8("clock");
  analogWrite(EN7, speed);
  analogWrite(EN8, speed);
}

void move_rotate_right(int speed) { // still not finished yet
  Serial.println("Rotating RIGHT");
  motor5("counter clock");
  motor6("counter clock");
  analogWrite(EN7, speed);
  analogWrite(EN8, speed);
}