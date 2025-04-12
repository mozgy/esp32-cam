#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <Arduino.h>
#include "esp_camera.h"

typedef const String picSizeStrings_t;
extern size_t photoFrameLength;
extern uint8_t imageRotation;

#include "camera_model.h"   // "esp_camera.h" and <Adafruit_BME280.h> - sensor_t clash
#include "camera_pins.h"

#define FLASH_LED LED_GPIO_NUM
// #define AI_THINKER_LED 33    // onboard red one
// #define LED_GPIO_NUM  4  // CAMERA_MODEL_AI_THINKER
// #define LED_GPIO_NUM 48  // CAMERA_MODEL_ESP32S3_CAM
// #define FLASH_ENABLE_HW true // not yet used

#include "variables.h"

extern bool flashEnabled;
extern bool SDCardOK;

void initCam( void );
void flashON( void );
void flashOFF( void );
void flashLED( uint32_t );
void doSnapPhoto( void );

/*
typedef enum {
    FRAMESIZE_96X96,    // 96x96
    FRAMESIZE_QQVGA,    // 160x120
    FRAMESIZE_QCIF,     // 176x144
    FRAMESIZE_HQVGA,    // 240x176
    FRAMESIZE_240X240,  // 240x240
    FRAMESIZE_QVGA,     // 320x240
    FRAMESIZE_CIF,      // 400x296
    FRAMESIZE_HVGA,     // 480x320
    FRAMESIZE_VGA,      // 640x480
    FRAMESIZE_SVGA,     // 800x600
    FRAMESIZE_XGA,      // 1024x768
    FRAMESIZE_HD,       // 1280x720
    FRAMESIZE_SXGA,     // 1280x1024
    FRAMESIZE_UXGA,     // 1600x1200
    // 3MP Sensors
    FRAMESIZE_FHD,      // 1920x1080
    FRAMESIZE_P_HD,     //  720x1280
    FRAMESIZE_P_3MP,    //  864x1536
    FRAMESIZE_QXGA,     // 2048x1536
    // 5MP Sensors
    FRAMESIZE_QHD,      // 2560x1440
    FRAMESIZE_WQXGA,    // 2560x1600
    FRAMESIZE_P_FHD,    // 1080x1920
    FRAMESIZE_QSXGA,    // 2560x1920
    FRAMESIZE_INVALID
} framesize_t;
 */

#endif
