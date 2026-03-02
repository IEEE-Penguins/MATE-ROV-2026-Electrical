#include "ESC.h"

ESCChannel::ESCChannel()
{
    channel_pin = ESC_1;
    set_signal = ESC_STOP;
    driver.setPeriodHertz(freq);
    driver.attach(channel_pin);
    arm();
}

ESCChannel::ESCChannel(byte pin, short signal)
{
    channel_pin = pin;
    set_signal = signal;
    driver.setPeriodHertz(freq);
    driver.attach(channel_pin);
    arm();
    drive(signal);
}

void ESCChannel::arm()
{
    driver.writeMicroseconds(stop_signal);
    last_signal = stop_signal;
    delay(3000);
}

void ESCChannel::drive(short signal)
{
    signal = signal > ESC_MAX_CC ? ESC_MAX_CC : (signal < ESC_MAX_CCW ? ESC_MAX_CCW : signal);
    driver.writeMicroseconds(signal);
    last_signal = signal;
}

void ESCChannel::stop()
{
    driver.writeMicroseconds(stop_signal);
    last_signal = stop_signal;
}