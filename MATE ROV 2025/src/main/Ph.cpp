
#include "Ph.h"

float get_ph(int pin, float temperature, DFRobot_PH &ph_object) {

  float voltage, phValue;
  static unsigned long timepoint = millis();
  if(millis() - timepoint > 1000U) {
      timepoint = millis();
      voltage = analogRead(pin) / 1024.0 * 5000;
      phValue = ph_object.readPH(voltage, temperature);
  }

  ph_object.calibration(voltage, temperature); 
  return phValue;
}