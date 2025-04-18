
#include <Arduino.h>

// #include "driver/rtc_io.h"

#include <LittleFS.h>
#include <FS.h>
#include <SD_MMC.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <ArduinoOTA.h>

#include "credentials.h"
#include "variables.h"
#include "camera.h"
#include "mywebserver.h"
#include "mywifi.h"

String httpUsernameStr;
String httpPasswordStr;
String wifiSSIDStr;
String wifiPasswordStr;

long timeZone = 1;
byte daySaveTime = 1;
struct tm startTime;

char elapsedTimeString[40];
char currentDateTime[17];

String cameraNameSuffix = CAM_SERIAL;

String photoFrame;
size_t photoFrameLength;
uint8_t imageRotation = 1; /* 1 - 0, 3 - 180, 6 - 90, 8 - 270 */

bool flashEnabled = FLASH_ENABLED;
bool timeLapse = TIME_LAPSE_MODE;
bool SDCardOK;

bool bme280Found;

bool prusaConnectActive = false;
String prusaHTMLResponse;
String prusaResponseCode;
String prusaTokenStr;
String camFingerPrintStr;

Ticker tickerCam, tickerBME, tickerPrusa;
boolean tickerCamFired, tickerBMEFired, tickerPrusaFired;
u_int32_t tickerCamCounter, tickerCamMissed;
u_int16_t timeLapseInterval = 60;
u_int16_t oldTickerValue;
u_int32_t tickerBMECounter, tickerBMEMissed;
u_int32_t tickerPrusaCounter, tickerPrusaMissed;
u_int16_t prusaConnectInterval = PRUSA_CONNECT_INTERVAL;

void funcCamTicker( void ) {
  tickerCamFired = true;
  tickerCamCounter++;
  tickerCamMissed++;
}

void funcBMETicker( void ) {
  tickerBMEFired = true;
  tickerBMECounter++;
  tickerBMEMissed++;
}

void funcPrusaTicker( void ) {
  tickerPrusaFired = true;
  tickerPrusaCounter++;
  tickerPrusaMissed++;
}

void prnEspStats( void ) {

  Serial.println();
  Serial.printf( "Sketch SW version: %s\n", SW_VERSION );
  Serial.printf( "Sketch size: %u\n", ESP.getSketchSize() );
  Serial.printf( "Sketch MD5: %u\n", ESP.getSketchMD5() );
  Serial.printf( "SDK: %s\n", ESP.getSdkVersion() );
  Serial.printf( "Free size: %u\n", ESP.getFreeSketchSpace() );
  Serial.printf( "Heap: %u\n", ESP.getFreeHeap() );
  Serial.printf( "CPU: %uMHz\n", ESP.getCpuFreqMHz() );
  Serial.printf( "ESP32 Chip: Model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision() );
  // Serial.printf( "Flash ID: %u\n", ESP.getFlashChipId() );
  Serial.printf( "Flash: Size %u Speed %u\n", ESP.getFlashChipSize(), ESP.getFlashChipSpeed() );
  Serial.printf( "PSRAM Size: %u\n", ESP.getPsramSize() );
  // Serial.printf( "Chip ID: %u\n", ESP.getChipId() ); // ESP8266 style
  uint64_t chipid = ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
  Serial.printf( "ESP32 Chip ID = %04X", (uint16_t)(chipid>>32) ); //print High 2 bytes
  Serial.printf( "%08X\n", (uint32_t)chipid );                     //print Low 4bytes.
//  Serial.printf( "Vcc: %u\n", ESP.getVcc() );
  Serial.println();

}

void fnElapsedStr( char *str ) {

  uint32_t sec = millis() / 1000;
  uint32_t minute = ( sec / 60 ) % 60;
  uint32_t hour = ( sec / 3600 ) % 24;
  uint32_t day = sec / 86400 ;

  sprintf( str, "Elapsed %u:%02u:%02u:%02u", day, hour, minute, ( sec % 60 ) );

  uint8_t strIndex = 8; // "Elapsed " == 8 chars

  while ( str[ strIndex ] == '0' || str[ strIndex ] == ':' ) {
    u_int8_t i = strIndex;
    while ( str[ i ] != '\0' ) {
      str[ i ] = str[ i + 1 ];
      i++;
    }
  }

  // borderline -> 0:00:00:00 -> ""

}

void getNTPTime( void ) {

  log_i( "Contacting Time Server - " );
  configTime( 3600*timeZone, daySaveTime*3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org" );
  // configTime( 3600*timeZone, daySaveTime*3600, "tik.t-com.hr", "tak.t-com.hr" ); // my local NTP sources
  delay( 2000 );
  startTime.tm_year = 0;
  getLocalTime( &startTime, 5000 );
  while( startTime.tm_year == 70 ) {
    log_e( "NTP failed, trying again .. " );
    delay( 5000 );
    getLocalTime( &startTime, 5000 );
  }
  log_i( "Local Time : %d-%02d-%02d %02d:%02d:%02d", (startTime.tm_year)+1900, (startTime.tm_mon)+1, startTime.tm_mday, startTime.tm_hour , startTime.tm_min, startTime.tm_sec );

//  sprintf( currentDateTime, "%04d", (tmstruct.tm_year)+1900 );
//  sprintf( currentDateTime, "%s%02d", currentDateTime, (tmstruct.tm_mon)+1 );
//  sprintf( currentDateTime, "%s%02d", currentDateTime, tmstruct.tm_mday );
//  sprintf( currentDateTime, "%s%02d", currentDateTime, tmstruct.tm_hour );
//  sprintf( currentDateTime, "%s%02d\0", currentDateTime, tmstruct.tm_min );

}

String getConfigValue( JsonDocument jsonConfig, String configName ) {

  String configValue = String( jsonConfig[ configName ] );
  if( configValue != "" ) {
    log_i( "Config %s=%s", configName, configValue );
  }
  return configValue;

}

bool getConfigBoolValue( JsonDocument jsonConfig, String configName ) {

  String configValue = String( jsonConfig[ configName ] );
  if( configValue != "" ) {
    log_i( "Config %s=%s", configName, configValue );
    if( configValue == "on" || configValue == "yes" )
      return true;
  }
  return false;

}

esp_err_t loadConfigFromSD( void ) {

  File configFP;
  JsonDocument cameraConfig;
  String configParam;

  if( !SDCardOK ) {
    log_e( "SD Card not found!" );
    return ESP_ERR_NOT_FOUND;
  }

  configFP = SD_MMC.open( "/config", FILE_READ );
  if( !configFP ) {
    log_e( "Cannot open config file!" );
    return ESP_FAIL;
  }

  DeserializationError jsonError = deserializeJson( cameraConfig, configFP );
  if( jsonError ) {
    log_e( "Failed to read file, using default configuration" );
    return ESP_FAIL;
  }

  // json {"http_username":"x","http_password":"y","wifi_ssid":"x","wifi_password":"y", \
  //       "camera_name":"cam_1","flash":"off","timelapse":"off", \
  //       "prusa_connect":"off","prusa_token":"xx","camera_fingerprint":"uuigen"}

  cameraNameSuffix = getConfigValue( cameraConfig, "camera_name" );
  flashEnabled = getConfigBoolValue( cameraConfig, "flash" );
  timeLapse = getConfigBoolValue( cameraConfig, "timelapse" );
  prusaConnectActive = getConfigBoolValue( cameraConfig, "prusa_connect" );
  prusaTokenStr = getConfigValue( cameraConfig, "prusa_token" );
  camFingerPrintStr = getConfigValue( cameraConfig, "camera_fingerprint" );
  wifiSSIDStr = getConfigValue( cameraConfig, "wifi_ssid" );
  wifiPasswordStr = getConfigValue( cameraConfig, "wifi_password" );
  httpUsernameStr = getConfigValue( cameraConfig, "http_username" );
  httpPasswordStr = getConfigValue( cameraConfig, "http_password" );

  return ESP_OK;

}

void initSDCard( void ) {

// bool setPins(int clk, int cmd, int d0);
// bool setPins(int clk, int cmd, int d0, int d1, int d2, int d3);
// bool SDMMCFS::begin(const char * mountpoint, bool mode1bit, bool format_if_mount_failed, int sdmmc_frequency, uint8_t maxOpenFiles)

  SDCardOK = true;
#ifdef CAMERA_MODEL_AI_THINKER
//  ESP32 (orig)
//  #define SD_MMC_CMD  15
//  #define SD_MMC_CLK  14
//  #define SD_MMC_D0   2
//  SD_MMC.setPins( SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0 );
#endif
#ifdef ARDUINO_ESP32S3_DEV
// #define XIAO_ESP32S3_SDCS_PIN 21
#define SD_MMC_CMD 38
#define SD_MMC_CLK 39
#define SD_MMC_D0  40
    log_i( "Remapping SD_MMC card pins to sd_clk=%d sd_cmd=%d sd_data=%d", SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0 );
    SD_MMC.setPins( SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0 );
#endif
  // if( !SD_MMC.begin() ) { // fast 4bit mode
  if( !SD_MMC.begin( "/sdcard", true ) ) { // slow 1bit mode
  // if( !SD_MMC.begin( "/sdcard", true, true ) ) { // slow 1bit mode, format
  // if( !SD_MMC.begin( "/sdcard", true, false, SDMMC_FREQ_DEFAULT, 5 ) ) {
  // if( !SD_MMC.begin( "/sdcard", true, false, 20000 ) ) {
#ifdef CAMERA_MODEL_AI_THINKER
  // pinMode(4, OUTPUT);
  // digitalWrite(4, 0); // set lamp pin fully off as sd_mmc library still initialises pin 4 in 1 line mode#else
#endif
    log_e( "SD card init failed" );
    SDCardOK = false;
    timeLapse = false;
    return;
  }

  uint8_t cardType = SD_MMC.cardType();

  if( cardType == CARD_NONE ) {
    log_e( "No SD card attached" );
    SDCardOK = false;
    timeLapse = false;
    return;
  }

  String SDCardType = "SD Card Type: ";
  if( cardType == CARD_MMC ) {
    SDCardType += "MMC";
  } else if( cardType == CARD_SD ) {
    SDCardType += "SDSC";
  } else if( cardType == CARD_SDHC ) {
    SDCardType += "SDHC";
  } else {
    SDCardType += "UNKNOWN";
  }

  uint64_t cardSize = SD_MMC.cardSize() / ( 1024 * 1024 );
  log_i( "%s, SD Card Size: %lluMB", SDCardType.c_str(), cardSize );

  if( !SD_MMC.mkdir( "/mozz-cam" ) ) {
    // log_e( "DIR exists .." );
  }

#ifdef CAMERA_MODEL_AI_THINKER
  flashLED( 50, true ); delay( 80 ); flashLED( 50, true ); delay( 80 ); flashLED( 50, true );
#endif

  return;

}

void setup() {

  Serial.begin( 115200 );
  // while( !Serial );   // FIXME - commented as it loops on XIAO S3 without USB data connection
  delay( 600 );
  // Serial.setDebugOutput( true );
  log_d( "Setup Start!" );
  // WiFi.printDiag(Serial); // CRASH - Guru Meditation Error

  // set these three lines above BEFORE AND AFTER the call to esp_wifi_init
  // esp_log_level_set("wifi", ESP_LOG_VERBOSE);
  // esp_wifi_internal_set_log_level(WIFI_LOG_VERBOSE);
  // esp_wifi_internal_set_log_mod(WIFI_LOG_MODULE_ALL, WIFI_LOG_SUBMODULE_ALL, true);

#ifdef CAMERA_MODEL_ESP32S3_CAM
//  neopixelWrite( FLASH_LED, (RGB_BRIGHTNESS >> 1), 0, 0 );
#endif

  prnEspStats();

  if( !LittleFS.begin( true ) ) {
    log_d( "Formatting LittleFS" );
    if( !LittleFS.begin() ) {
      log_e( "An Error has occurred while mounting LittleFS" );
    }
  }

#ifdef CAMERA_MODEL_ESP32S3_CAM
  neopixelWrite( FLASH_LED, 0, 0, RGB_BRIGHTNESS );
#endif

#ifdef HAVE_SDCARD
  log_d( "Before initSDCard!" );
  // [E][SD_MMC.cpp:132] begin(): Failed to mount filesystem. If you want the card to be formatted, set format_if_mount_failed = true.
  initSDCard();  // *HAS* to be *before* initCam() if board has SDCard !
  loadConfigFromSD();
#endif

#ifdef CAMERA_MODEL_ESP32S3_CAM
  neopixelWrite( FLASH_LED, 0, RGB_BRIGHTNESS, 0 );
#endif

#ifdef HAVE_CAMERA
  log_d( "Before initCam!" );
  initCam();  // *HAS* to be *after* initSDCard() if board has SDCard !
#endif

#if !defined(CAMERA_MODEL_AI_THINKER) && !defined (CAMERA_MODEL_ESP32S3_CAM)
  flashEnabled = false;
#endif

  log_d( "Before initWiFi!" );
  wifiSSIDStr = "" + String( wifi_ssid );;
  wifiPasswordStr = "" + String( wifi_password );;
  initWiFi();
  log_d( "Before NTP!" );
  getNTPTime();

#ifdef CAMERA_MODEL_ESP32S3_CAM
  neopixelWrite( FLASH_LED, 0, 0, 0 );
#endif

#ifdef HAVE_BME280
  log_d( "Before initBME!" );
  initBME();  // *HAS* to be *after* initSDCard() if board has SDCard ! Hmm, maybe not before
#endif

  log_d( "Before initAsyncWebServer!" );
  httpUsernameStr = "" + String( http_username );
  httpPasswordStr = "" + String( http_password );
  initAsyncWebServer();
  log_d( "Before initOTA!" );
  initOTA();

  tickerCamCounter = 0;
  tickerCamMissed = 0;
  tickerCam.attach( timeLapseInterval, funcCamTicker );
  tickerCamFired = true;
  oldTickerValue = timeLapseInterval;

#ifdef PRUSA_CONNECT
  tickerPrusaCounter = 0;
  tickerPrusaMissed = 0;
  tickerPrusa.attach( PRUSA_CONNECT_INTERVAL, funcPrusaTicker );
  tickerPrusaFired = true;
#endif

#ifdef HAVE_BME280
  tickerBMECounter = 0;
  tickerBMEMissed = 0;
  tickerBME.attach( 61, funcBMETicker );
  tickerBMEFired = true;
  bme280Found = false;
#endif

}

void loop() {

  ArduinoOTA.handle();

  if( tickerCamFired ) {
    tickerCamFired = false;
#ifdef HAVE_CAMERA
    doSnapSavePhoto();
#endif
    int wifiStatus = WiFi.status();
    log_i( "%s, (RSSI): %d dBm, MAC=%s, IP=%s", get_wifi_status( wifiStatus ), WiFi.RSSI(), WiFi.macAddress().c_str(), ( WiFi.status() == WL_CONNECTED ) ? WiFi.localIP().toString().c_str() : "" );
    fnElapsedStr( elapsedTimeString );
    log_d( "%s - Startup Time : %d-%02d-%02d %02d:%02d:%02d", elapsedTimeString, (startTime.tm_year)+1900, (startTime.tm_mon)+1, startTime.tm_mday, startTime.tm_hour , startTime.tm_min, startTime.tm_sec );
    if( oldTickerValue != timeLapseInterval ) {
      tickerCam.detach( );
      tickerCam.attach( timeLapseInterval, funcCamTicker );
      oldTickerValue = timeLapseInterval;
    }
    if( tickerCamMissed > 1 ) {
      log_e( "Missed %d tickers", tickerCamMissed - 1 );
    }
    tickerCamMissed = 0;
#ifdef ARDUINO_ESP32S3_DEV
    float temp_celsius = temperatureRead();
    log_i( "Temp onBoard %.2f°C", temp_celsius );
#endif
  }

#ifdef HAVE_BME280
  if( tickerBMEFired ) {
    tickerBMEFired = false;

    fnElapsedStr( elapsedTimeString );
    if( bme280Found ) {
      bmeSerialPrint();
    }
    log_d( "BME280 tick - %s", elapsedTimeString );

    if( tickerBMEMissed > 1 ) {
      log_e( "Missed %d tickers", tickerBMEMissed - 1 );
    }
    tickerBMEMissed = 0;

  }
#endif

#ifdef PRUSA_CONNECT
  if( tickerPrusaFired ) {
    tickerPrusaFired = false;

    fnElapsedStr( elapsedTimeString );
    log_d( "PrusaConnect tick - %s", elapsedTimeString );

    prusaHTMLResponse = photoSendPrusaConnect();
    log_d( "PrusaConnect response - %s", prusaHTMLResponse.c_str() );
    JsonDocument prusaHTMLJSONResponse;
    DeserializationError jsonError = deserializeJson( prusaHTMLJSONResponse, prusaHTMLResponse );
    if( jsonError ) {
      log_e( "Failed to parse PrusaConnect HTML Response!" );
    }
    prusaResponseCode = String( prusaHTMLJSONResponse["status_code"] );
    if( prusaResponseCode == "" ) {
      prusaResponseCode = "Error (ToDo)";
    }

    if( tickerPrusaMissed > 1 ) {
      log_e( "Missed %d tickers", tickerPrusaMissed - 1 );
    }
    tickerPrusaMissed = 0;

  }
#endif

}
