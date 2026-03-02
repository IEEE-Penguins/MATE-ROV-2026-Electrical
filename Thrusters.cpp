#include "Thrusters.h"

Thrusters::Thrusters()
{
    escs[0] = ESCChannel(ESC_1, ESC_STOP);
    escs[1] = ESCChannel(ESC_2, ESC_STOP);
    escs[2] = ESCChannel(ESC_3, ESC_STOP);
    escs[3] = ESCChannel(ESC_4, ESC_STOP);
    escs[4] = ESCChannel(ESC_5, ESC_STOP);
    escs[5] = ESCChannel(ESC_6, ESC_STOP);
}

Thrusters::Thrusters(ESCChannel esc_channels[ESCs_CHANNELS_NUM])
{
    for (byte i = 0; i < ESCs_CHANNELS_NUM; i++)
        escs[i] = esc_channels[i];
}

void Thrusters::drive(float speeds[ESCs_CHANNELS_NUM])
{
    for (byte i = 0; i < ESCs_CHANNELS_NUM; i++)
    {
        ESC_signals[i] = mapSpeed(speeds[i]);
        escs[i].drive(ESC_signals[i]);
    }
}
