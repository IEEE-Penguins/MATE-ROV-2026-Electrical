#include <thrusters.h>

ThrustersController thrustersController;


float speed[29][5] = {
  //~ m1&2
  {0.3, 0.3, 0.3, 0.3, 0.3},
  {-0.3, -0.3, -0.3, -0.3, -0.3},
  // {0.5, 0.0, 0.0, 0.0, 0.0},
  // {0.0, -0.5, -0.5, -0.5, -0.5},
  {0.0, 0.0, 0.0, 0.0, 0.0}

};

void setup() {
  thrustersController.begin();
  Serial.begin(115200);
}

void loop() {

  delay(50);

  for(int i = 0; i < 3; i++) {
    thrustersController.drive(speed[i]);
    Serial.print("["); Serial.print(i+1); Serial.print("]: ");
    delay(2000);
  }

  delay(50);
  
}