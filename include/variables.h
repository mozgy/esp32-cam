#ifndef _MYVARS_H_
#define _MYVARS_H_

#include <Arduino.h>

#define SW_VERSION "0.33.3"

#define HAVE_CAMERA
#define ESP_CAM_HOSTNAME "mozz-cam"
#define CAM_SERIAL "1"

#define FLASH_ENABLED false

#define HAVE_SDCARD
#define TIME_LAPSE_MODE false
#define HIDE_ROOT_DIR false

#undef HAVE_BME280
#define SEALEVELPRESSURE_HPA (1013.25)

#undef PRUSA_CONNECT
#define PRUSA_CONNECT_INTERVAL 75

extern String cameraNameSuffix;

extern String photoFrame;
extern bool timeLapse;
extern int intervalTimeLapse;

extern long timeZone;
extern byte daySaveTime;

extern char elapsedTimeString[40];
extern char currentDateTime[17];

extern void bmeSerialPrint( void );
extern void initBME( void );
extern bool bme280Found;

#endif
