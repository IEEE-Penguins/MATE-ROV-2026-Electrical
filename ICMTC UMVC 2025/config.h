#ifndef CONFIG_H
#define CONFIG_H


/*
    --<ESCs pins>--
*/
#define ESC_1_2 15 // 1,2
#define ESC_3 2 // 3
#define ESC_4 4 // 4
#define ESC_5 14 // 5
#define ESC_6 33 // 6

/*
    --<number of esc's channels used>--
    5 is the maximum number, if changed to 1, only ESC_1_2 will output a signal
    if changed to 2, only ESC_1_2 and ESC_3_4 will output a signal, etc...
*/
#define ESCs_CHANNELS_NUM 5

/*
    --<servo min, max and stop pulse values>--
*/
#define ESC_MAX_CC 1900
#define ESC_MAX_CCW 1100
#define ESC_STOP 1500

/*
    --<number of servo's channels used>--
    16 is the maximum number, if changed to 1, only channel 1 will output a signal
    if changed to 2, only channel 1 and channel 2will output a signal, etc...
*/
#define SERVOS_CHANNELS_NUM 4
//#define OPEN_LOOP_STOP 90
//#define SERVO_CC 0
//#define SERVO_CCW 180

/*
    --<servo min, max and stop pulse values>--
*/
#define SERVO_MIN 100  	//-> open
#define SERVO_MAX 540   //-> close
#define SERVO_STEP 10

/*
    --<lights>--
*/
#define EXTERNAL_LIGHTS 32


#endif
