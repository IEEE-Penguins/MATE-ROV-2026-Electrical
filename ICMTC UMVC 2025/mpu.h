#ifndef MPU_H
#define MPU_H

#include <stdint.h>

struct MPUValues {
  float acc_x, acc_y, acc_z;
  float gyro_x, gyro_y, gyro_z;
  float angle_x, angle_y;
};

class MPU {
  public:
    void begin();
    MPUValues read();

  private:
    float smooth(float current, float previous, float factor, float threshold);

    int16_t accel[3] = {0};
    int16_t gyro[3] = {0};

    float filtered_roll = -2.5;
    float filtered_pitch = -7.5;
    float filtered_yaw = 1.0;  // not used here but useful later
};

#endif
