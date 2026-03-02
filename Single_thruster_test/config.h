#ifndef CONFIG_H
#define CONFIG_H

/*
    --<ESCs pins>--
*/
#define ESC_1 15
#define ESC_2 16
#define ESC_3 2
#define ESC_4 4
#define ESC_5 14
#define ESC_6 33

#define ESCs_CHANNELS_NUM 6

#define ESC_MAX_CC 1900  // 1900 >> 389 PWM
#define ESC_MAX_CCW 1100 // 1100 >> 225 PWM
#define ESC_STOP 1500    // 1500 >> 307 PWM

#define SERVOS_CHANNELS_NUM 4

#define SERVO_MIN 100
#define SERVO_MAX 540
#define SERVO_STEP 10

#define SERVO_TYPE 0
#define THRUSTER_TYPE 1

#define EXTERNAL_LIGHTS 25

#define LEAKAGE_PIN 32

#endif