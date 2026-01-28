
const int leakSensorPin = 4;  
const int ledPin = 2;         

void setup() {
 
  Serial.begin(115200);
  
  
  pinMode(leakSensorPin, INPUT);
  

  pinMode(ledPin, OUTPUT);
  
  Serial.println("System Started: Monitoring for leaks...");
}

void loop() {
  
  int sensorState = digitalRead(leakSensorPin);

  if (sensorState == HIGH) {
 
    Serial.println(" ALERT: Leak Detected!");
    digitalWrite(ledPin, HIGH); 
  } else {
    
    digitalWrite(ledPin, LOW);
    Serial.println(" ALERT: Leak NOT Detected!");  
  }

 
  delay(200); 
}