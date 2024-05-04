#ifndef _MYWIFI_H_
#define _MYWIFI_H_

#include "esp_wifi.h"
#include <WiFi.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include "soc/rtc_cntl_reg.h"

#include "variables.h"

// #define CONFIG_ESP_WIFI_SOFTAP_SUPPORT 0
// #define CONFIG_ESP_WIFI_ENTERPRISE_SUPPORT 0

#define WIFI_DISC_DELAY 30000L
extern unsigned long wifiWaitTime;
extern int wifiConnTries;

extern void flashLED( uint32_t );
extern void flashLED( uint32_t, bool );

void waitForConnect ( unsigned long timeout );
String get_wifi_status( int status );
void WiFiStationConnected( WiFiEvent_t event, WiFiEventInfo_t info );
void WiFiGotIP( WiFiEvent_t event, WiFiEventInfo_t info );
void WiFiStationDisconnected( WiFiEvent_t event, WiFiEventInfo_t info );
void initWiFi( void );

#endif
