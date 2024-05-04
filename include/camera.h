#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <Arduino.h>
#include "esp_camera.h"

typedef const String picSizeStrings_t;
extern size_t photoFrameLength;

// foo has to be of framesize_t size
const picSizeStrings_t foo[] = {
  "Framesize 96x96",
  "Framesize QQVGA - 160x120",
  "Framesize QCIF - 176x144",
  "Framesize HQVGA - 240x176",
  "Framesize 240x240",
  "Framesize QVGA - 320x240",
  "Framesize CIF - 400x296",
  "Framesize HVGA - 480x320",
  "Framesize VGA - 640x480",
  "Framesize SVGA - 800x600",
  "Framesize XGA - 1024x768",
  "Framesize HD",
  "Framesize SXGA - 1280x1024",
  "Framesize UXGA - 1600x1200",
  "Framesize FHD",
  "Framesize P_HD",
  "Framesize P_3MP",
  "Framesize QXGA - 2048x1536"
};

// Select camera model
// #define CAMERA_MODEL_WROVER_KIT
// #define CAMERA_MODEL_ESP_EYE
// #define CAMERA_MODEL_M5STACK_PSRAM
// #define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER
// #define CAMERA_MODEL_XIAO_ESP32S3
#include "camera_pins.h"

#define FLASH_LED LED_GPIO_NUM
// #define AI_THINKER_LED 33    // onboard red one
// #define FLASH_ENABLE_HW true // not yet used

#define XIAO_ESP32S3_SDCS_PIN 21

#include "variables.h"

// extern int picBrightness;
// extern int picSaturation;
extern bool flashEnabled;
extern bool SDCardOK;

void initCam( void );
// void fnSetFrameSize( String frameSize );
void flashON( void );
void flashOFF( void );
void flashLED( uint32_t );
void doSnapPhoto( void );

#endif

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
