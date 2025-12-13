#include <servos.h>

ServosController servosController;

void setup() {
  Serial.begin(115200);
  servosController.begin();
}

int dir[8][4] = {
  {1, 0, 0, 0},
  {-1, , 0, 0},
  {0, 1, 0, 0},
  {0, -1, 0, 0},
  {0, 0, 1, 0},
  {0, 0, -1, 0},
  {1, 1, 1, 0},
  {-1, -1, -1, 0}
}

void loop() {
  
  delay(50);
    if(!digitalRead(CW_BTN)) servosController.drive(0, 1);
    else if(!digitalRead(CCW_BTN)) servosController.drive(0, -1);
  delay(50);
  
}