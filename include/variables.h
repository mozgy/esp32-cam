#ifndef _MYVARS_H_
#define _MYVARS_H_

#include <Arduino.h>

#define SW_VERSION "0.30.2"

#define HAVE_CAMERA
#define ESP_CAM_HOSTNAME "mozz-cam"
#define CAM_SERIAL "2"

#define PRUSA_CONNECT
#define PRUSA_CONNECT_INTERVAL 90

#define FLASH_ENABLED false

#define HAVE_SDCARD
#define TIME_LAPSE_MODE false
#define HIDE_ROOT_DIR false

#undef HAVE_BME280
#define SEALEVELPRESSURE_HPA (1013.25)

extern String photoFrame;
extern bool timeLapse;

extern long timeZone;
extern byte daySaveTime;

extern int waitTime;

extern char elapsedTimeString[40];
extern char currentDateTime[17];

extern void bmeSerialPrint( void );
extern void initBME( void );
extern bool bme280Found;

extern void initOTA( void );

#endif
