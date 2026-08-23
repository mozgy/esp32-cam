#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <Arduino.h>
#include "esp_camera.h"

// Select (only one) camera model - *before* camera_pins.h
// #define CAMERA_MODEL_ESP_EYE
#define CAMERA_MODEL_AI_THINKER
// #define CAMERA_MODEL_XIAO_ESP32S3
// #define CAMERA_MODEL_ESP32S3_EYE
// #define CAMERA_MODEL_ESP32S3_CAM
  // #define CAMERA_MODEL_ESP32S3_CAM_VARIANT_1
  // #define CAMERA_MODEL_ESP32S3_CAM_VARIANT_2
  // #define CAMERA_MODEL_ESP32S3_CAM_VARIANT_3
// #define CAMERA_MODEL_Waveshare_ESP32S3_CAM
// Select (only one) camera model - *before* camera_pins.h

#include "camera_pins.h"
#include "variables.h"

#ifdef CAMERA_MODEL_AI_THINKER
  #undef FLASH_NEOPIXEL
  #undef REVERSE_PULLUP
  #define FLASH_LED LED_GPIO_NUM
  #define AI_THINKER_LED 33    // onboard red one
#endif
#ifdef CAMERA_MODEL_ESP32S3_CAM
  #if defined(CAMERA_MODEL_ESP32S3_CAM_VARIANT_1)
    #define FLASH_NEOPIXEL
    #undef REVERSE_PULLUP
    #define FLASH_LED LED_GPIO_NUM
  #elif defined(CAMERA_MODEL_ESP32S3_CAM_VARIANT_2)
    #undef FLASH_NEOPIXEL
    #define REVERSE_PULLUP
    #define FLASH_LED 1
  #elif defined(CAMERA_MODEL_ESP32S3_CAM_VARIANT_3)
    #undef FLASH_NEOPIXEL
    #undef REVERSE_PULLUP
    #define FLASH_LED LED_GPIO_NUM
#endif
#ifdef CAMERA_MODEL_Waveshare_ESP32S3_CAM
  #undef FLASH_NEOPIXEL
  #undef REVERSE_PULLUP
  #undef FLASH_LED
#endif


typedef const String picSizeStrings_t;
extern size_t photoFrameLength;
extern struct tm photoSnapTime;
extern uint8_t imageRotation;

extern bool flashEnabled;
extern bool SDCardOK;

extern volatile uint32_t asyncStreamClients;

void initCam( void );
void flashON( void );
void flashOFF( void );
void flashLED( uint32_t );
void flashLEDatStart( void );
void flashLEDafterFS( void );
void flashLEDafterInitSD( void );
void flashLEDafterInitWiFi( void );
void flashLEDafterInitWeb( void );
void flashLEDstreamON( void );
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
