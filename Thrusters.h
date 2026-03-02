#ifndef THRUSTERS_H
#define THRUSTERS_H

#include "config.h"
#include "ESC.h"
#include <Arduino.h>

class Thrusters
{
private:
    ESCChannel escs[ESCs_CHANNELS_NUM];
    short ESC_signals[ESCs_CHANNELS_NUM];
    // -1 to 0 >> 1100 to 1500
    // 0 to 1 >> 1500 to 1900
    int mapSpeed(float x)
    {
        return (int)((x - -1.0) * (float)(ESC_MAX_CC - ESC_MAX_CCW) / (1.0 - -1.0) + ESC_MAX_CCW);
    }

public:
    Thrusters();
    Thrusters(ESCChannel esc_channels[ESCs_CHANNELS_NUM]);
    void drive(float speeds[ESCs_CHANNELS_NUM]);
    void stopAll();
};

#endif // THRUSTERS_H