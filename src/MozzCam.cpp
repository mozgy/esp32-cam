
#include <Arduino.h>

// #include "driver/rtc_io.h"

#include <LittleFS.h>
#include <FS.h>
#include <SD_MMC.h>
#include <Ticker.h>
#include <ArduinoOTA.h>

#include "credentials.h"
#include "variables.h"
#include "camera.h"
#include "mywebserver.h"
#include "mywifi.h"

long timeZone = 1;
byte daySaveTime = 1;
struct tm startTime;

char elapsedTimeString[40];
char currentDateTime[17];

String photoFrame;
size_t photoFrameLength;

bool flashEnabled = FLASH_ENABLED;
bool timeLapse = TIME_LAPSE_MODE;
bool SDCardOK;

bool bme280Found;

Ticker tickerCam, tickerBME, tickerPrusa;
boolean tickerCamFired, tickerBMEFired, tickerPrusaFired;
int tickerCamCounter, tickerCamMissed;
int waitTime = 60;
int oldTickerValue;
int tickerBMECounter, tickerBMEMissed;
int tickerPrusaCounter, tickerPrusaMissed;

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

  uint64_t chipid;

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
  // uint64_t EspClass::getEfuseMac(void)
  chipid = ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
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
  delay( 400 );
  Serial.setDebugOutput( true );
  log_d( "Setup Start!" );
  // WiFi.printDiag(Serial); // research this

  // set these three lines above BEFORE AND AFTER the call to esp_wifi_init
  // esp_log_level_set("wifi", ESP_LOG_VERBOSE);
  // esp_wifi_internal_set_log_level(WIFI_LOG_VERBOSE);
  // esp_wifi_internal_set_log_mod(WIFI_LOG_MODULE_ALL, WIFI_LOG_SUBMODULE_ALL, true);

#ifdef CAMERA_MODEL_ESP32S3_CAM
  neopixelWrite( FLASH_LED, RGB_BRIGHTNESS, 0, 0 );
#endif

  prnEspStats();

  if( !LittleFS.begin( true ) ) {
    log_d( "Formatting LittleFS" );
    if( !LittleFS.begin( ) ) {
      log_e( "An Error has occurred while mounting LittleFS" );
    }
  }

  log_d( "Before initWiFi!" );
  delay( 10 );
  initWiFi();
  wifiWaitTime = millis();
  getNTPTime();

#ifdef CAMERA_MODEL_ESP32S3_CAM
  neopixelWrite( FLASH_LED, 0, 0, RGB_BRIGHTNESS );
#endif

#ifdef HAVE_SDCARD
  log_d( "Before initSDCard!" );
  delay( 10 );
  // [E][SD_MMC.cpp:132] begin(): Failed to mount filesystem. If you want the card to be formatted, set format_if_mount_failed = true.
  initSDCard( );  // *HAS* to be *before* initCam() if board has SDCard !
#endif

#ifdef CAMERA_MODEL_ESP32S3_CAM
  neopixelWrite( FLASH_LED, 0, 0, 0 );
#endif

#ifdef HAVE_CAMERA
  log_d( "Before initCam!" );
  delay( 10 );
  initCam();  // *HAS* to be *after* initSDCard() if board has SDCard !
#endif

#if !defined(CAMERA_MODEL_AI_THINKER) && !defined (CAMERA_MODEL_ESP32S3_CAM)
  flashEnabled = false;
#endif

#ifdef HAVE_BME280
  log_d( "Before initBME!" );
  delay( 10 );
  initBME();  // *HAS* to be *after* initSDCard() if board has SDCard ! Hmm, maybe not before
#endif

  delay( 10 );
  log_d( "Before initAsyncWebServer!" );
  initAsyncWebServer();
  log_d( "Before initOTA!" );
  initOTA();

  tickerCamCounter = 0;
  tickerCamMissed = 0;
  tickerCam.attach( waitTime, funcCamTicker );
  tickerCamFired = true;
  oldTickerValue = waitTime;

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

  ArduinoOTA.handle();  // FIXME - OTA not working for over #1000000 firmware.bin size

  if( tickerCamFired ) {
    int wifiStatus;
    tickerCamFired = false;
#ifdef HAVE_CAMERA
    doSnapSavePhoto();
#endif
    wifiStatus = WiFi.status();
    log_d( "%s", get_wifi_status( wifiStatus ) );
    log_i( "(RSSI): %d dBm, MAC=%s", WiFi.RSSI(), WiFi.macAddress().c_str() );
    if( WiFi.status() == WL_CONNECTED ) {
      log_i( "IP=%s", WiFi.localIP().toString().c_str() );   // TODO - add to previous log line
    }
    fnElapsedStr( elapsedTimeString );
    log_d( "%s - Startup Time : %d-%02d-%02d %02d:%02d:%02d", elapsedTimeString, (startTime.tm_year)+1900, (startTime.tm_mon)+1, startTime.tm_mday, startTime.tm_hour , startTime.tm_min, startTime.tm_sec );
    if( oldTickerValue != waitTime ) {
      tickerCam.detach( );
      tickerCam.attach( waitTime, funcCamTicker );
      oldTickerValue = waitTime;
    }
    if( tickerCamMissed > 1 ) {
      log_e( "Missed %d tickers", tickerCamMissed - 1 );
    }
    tickerCamMissed = 0;
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

    String htmlResponse = photoSendPrusaConnect();
    log_d( "PrusaConnect response - %s", htmlResponse.c_str() );

    if( tickerPrusaMissed > 1 ) {
      log_e( "Missed %d tickers", tickerPrusaMissed - 1 );
    }
    tickerPrusaMissed = 0;

  }
#endif

}
