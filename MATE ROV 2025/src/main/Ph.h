
#ifndef PH_H
#define PH_H

#include <Arduino.h>
#include "DFRobot_PH.h"
// #include <EEPROM.h>

float get_ph(int pin, float temperature, DFRobot_PH &ph_object);

#endif