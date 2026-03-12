
// const int leakSensorPin = 4;  
// const int ledPin = 2;         

// void setup() {
 
//   Serial.begin(115200);
  
  
//   pinMode(leakSensorPin, INPUT);
  

//   pinMode(ledPin, OUTPUT);
  
//   Serial.println("System Started: Monitoring for leaks...");
// }

// void loop() {
  
//   int sensorState = digitalRead(leakSensorPin);

//   if (sensorState == HIGH) {
 
//     Serial.println(" ALERT: Leak Detected!");
//     digitalWrite(ledPin, HIGH); 
//   } else {
    
//     digitalWrite(ledPin, LOW);
//     Serial.println(" ALERT: Leak NOT Detected!");  
//   }

 
//   delay(200); 
// }


const int ledPin = 16;         
const int leakSensorPin = 25;  

void setup() {
 pinMode(ledPin, OUTPUT);
 pinMode(leakSensorPin, INPUT);
  Serial.begin(9600);

}

void loop() {
  bool button_state = digitalRead(leakSensorPin);
  if (button_state == HIGH)
  {
    Serial.println("High");
    digitalWrite(ledPin, HIGH);  
  }
  else
  {

    Serial.println("Low");
    digitalWrite(ledPin, LOW); 
  }
}