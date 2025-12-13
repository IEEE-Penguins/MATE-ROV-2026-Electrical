
#include "PH.h"

float get_ph(int pin, float temperature, DFRobot_PH ph_object) {
  static unsigned long timepoint = millis();
  if(millis()-timepoint>1000U){                 
      timepoint = millis();
      float voltage = analogRead(PH_PIN)/1024.0*5000;
      float phValue = ph_object.readPH(voltage,temperature);
      return phValue;
  }
  ph_object.calibration(voltage,temperature); 
}