#ifndef MAG_H
#define MAG_H

#include <../config.h>
#include <Arduino.h>
#include <Wire.h>

typedef struct {
  float angle_x;
  float angle_y;
  float angle_z;
} MAGValues;

class MAG {
  public:
    void begin();
    MAGValues read();
    
  private:
    //[?]- define any internal values/objects/methods here

};

#endif  