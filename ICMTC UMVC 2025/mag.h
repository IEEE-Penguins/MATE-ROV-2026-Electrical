#ifndef MAG_H
#define MAG_H

class MAG {
  public:
    void begin();
    float read();  // return only yaw heading
};

#endif
