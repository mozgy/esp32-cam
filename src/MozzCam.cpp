
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

Ticker tickerCam, tickerBME;
boolean tickerCamFired, tickerBMEFired;
int tickerCamCounter, tickerCamMissed;
int waitTime = 60;
int oldTickerValue;
int tickerBMECounter, tickerBMEMissed;

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

void prnEspStats( void ) {

  uint64_t chipid;

  Serial.println();
  Serial.printf( "Sketch SW version: %s\n", SW_VERSION );
  Serial.printf( "Sketch size: %u\n", ESP.getSketchSize() );
  Serial.printf( "Sketch MD5: %u\n", ESP.getSketchMD5() );
  Serial.printf( "Free size: %u\n", ESP.getFreeSketchSpace() );
  Serial.printf( "Heap: %u\n", ESP.getFreeHeap() );
  Serial.printf( "CPU: %uMHz\n", ESP.getCpuFreqMHz() );
  Serial.printf( "Chip Revision: %u\n", ESP.getChipRevision() );
  Serial.printf( "SDK: %s\n", ESP.getSdkVersion() );
//  Serial.printf( "Flash ID: %u\n", ESP.getFlashChipId() );
  Serial.printf( "Flash Size: %u\n", ESP.getFlashChipSize() );
  Serial.printf( "Flash Speed: %u\n", ESP.getFlashChipSpeed() );
  Serial.printf( "PSRAM Size: %u\n", ESP.getPsramSize() );
//  Serial.printf( "Chip ID: %u\n", ESP.getChipId() );
  chipid=ESP.getEfuseMac(); //The chip ID is essentially its MAC address(length: 6 bytes).
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

void initOTA( void ) {

  // Port defaults to 3232
  // ArduinoOTA.setPort( 3232 );

  // Hostname defaults to esp3232-[MAC]
  String hostName;
  hostName = ESP_CAM_HOSTNAME;
  hostName += "-";
  hostName += CAM_SERIAL;
  ArduinoOTA.setHostname( hostName.c_str() );

  // No authentication by default
  // ArduinoOTA.setPassword( "admin" );

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash( "21232f297a57a5a743894a0e4a801fc3" );

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // Stop the camera since OTA will crash the module if it is running.
      // the unit will need rebooting to restart it, either by OTA on success, or manually by the user
      Serial.println("Stopping Camera");
      esp_err_t err = esp_camera_deinit();
      // critERR = "<h1>OTA Has been started</h1><hr><p>Camera has Halted!</p>";
      // critERR += "<p>Wait for OTA to finish and reboot, or <a href=\"control?var=reboot&val=0\" title=\"Reboot Now (may interrupt OTA)\">reboot manually</a> to recover</p>";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

}

void getNTPTime( void ) {

  Serial.print( "Contacting Time Server - " );
//  configTime( 3600*timeZone, daySaveTime*3600, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org" );
  configTime( 3600*timeZone, daySaveTime*3600, "tik.t-com.hr", "tak.t-com.hr" );
  delay( 2000 );
  startTime.tm_year = 0;
  getLocalTime( &startTime, 5000 );
  while( startTime.tm_year == 70 ) {
    Serial.print( "NTP failed, trying again .. " );
    delay( 5000 );
    getLocalTime( &startTime, 5000 );
  }
  Serial.printf( "Local Time : %d-%02d-%02d %02d:%02d:%02d\n", (startTime.tm_year)+1900, (startTime.tm_mon)+1, startTime.tm_mday, startTime.tm_hour , startTime.tm_min, startTime.tm_sec );

//  // yes, I know it can be oneliner -
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
    Serial.printf( "Remapping SD_MMC card pins to sd_clk=%d sd_cmd=%d sd_data=%d\n", SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0 );
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
    Serial.println( "SD card init failed" );
    SDCardOK = false;
    timeLapse = false;
    return;
  }

  uint8_t cardType = SD_MMC.cardType();

  if( cardType == CARD_NONE ) {
    Serial.println( "No SD card attached" );
    return;
  }

  Serial.print( "SD Card Type: " );
  if( cardType == CARD_MMC ) {
    Serial.println( "MMC" );
  } else if( cardType == CARD_SD ) {
    Serial.println( "SDSC" );
  } else if( cardType == CARD_SDHC ) {
    Serial.println( "SDHC" );
  } else {
    Serial.println( "UNKNOWN" );
  }

  uint64_t cardSize = SD_MMC.cardSize() / ( 1024 * 1024 );
  Serial.printf( "SD Card Size: %lluMB\n", cardSize );

  if( !SD_MMC.mkdir( "/mozz-cam" ) ) {
    // Serial.println( "DIR exists .." );
  }

#ifdef CAMERA_MODEL_AI_THINKER
  flashLED( 50, true ); delay( 80 ); flashLED( 50, true ); delay( 80 ); flashLED( 50, true );
#endif

  return;

}

void setup() {

  Serial.begin( 115200 );
  while( !Serial );   // TODO - rework all Serial outputs to log_info, log_error, log_debug
  delay( 400 );
  Serial.println( "Setup Start!" );
  Serial.setDebugOutput( true );
  // WiFi.printDiag(Serial); // research this

  // set these three lines above BEFORE AND AFTER the call to esp_wifi_init
  // esp_log_level_set("wifi", ESP_LOG_VERBOSE);
  // esp_wifi_internal_set_log_level(WIFI_LOG_VERBOSE);
  // esp_wifi_internal_set_log_mod(WIFI_LOG_MODULE_ALL, WIFI_LOG_SUBMODULE_ALL, true);

  prnEspStats();

  if( !LittleFS.begin( true ) ) {
    Serial.println( "Formatting LittleFS" );
    if( !LittleFS.begin( ) ) {
      Serial.println( "An Error has occurred while mounting LittleFS" );
    }
  }

  Serial.println( "Before initWiFi!" );
  delay( 10 );
  initWiFi();
  wifiWaitTime = millis();
  getNTPTime();
  Serial.println( WiFi.localIP() );

#ifdef HAVE_SDCARD
  Serial.println( "Before initSDCard!" );
  delay( 10 );
  // [E][SD_MMC.cpp:132] begin(): Failed to mount filesystem. If you want the card to be formatted, set format_if_mount_failed = true.
  initSDCard( );  // *HAS* to be *before* initCam() if board has SDCard !
#endif

#ifndef CAMERA_MODEL_AI_THINKER
  // using LED only on ESP32-CAM (originaly made by AI-Thinker)
  flashEnabled = false;
#endif

#ifdef HAVE_CAMERA
  Serial.println( "Before initCam!" );
  delay( 10 );
  initCam();  // *HAS* to be *after* initSDCard() if board has SDCard !
#endif

#ifdef HAVE_BME280
  Serial.println( "Before initBME!" );
  delay( 10 );
  initBME();  // *HAS* to be *after* initSDCard() if board has SDCard ! Hmm, maybe not before
#endif

  delay( 10 );
  Serial.println( "Before initAsyncWebServer!" );
  initAsyncWebServer();
  Serial.println( "Before initOTA!" );
  initOTA();

  tickerCamCounter = 0;
  tickerCamMissed = 0;
  tickerCam.attach( waitTime, funcCamTicker );
  tickerCamFired = true;
  oldTickerValue = waitTime;

  tickerBMECounter = 0;
  tickerBMEMissed = 0;
  tickerBME.attach( 61, funcBMETicker );
  tickerBMEFired = true;

  bme280Found = false;

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
    Serial.println( get_wifi_status( wifiStatus ) );
    Serial.print( "(RSSI): " );
    Serial.print( WiFi.RSSI() );
    Serial.print( " dBm, MAC=" );
    Serial.print( WiFi.macAddress() );
    if( WiFi.status() == WL_CONNECTED ) {
      Serial.print( ", IP=" );
      Serial.print( WiFi.localIP() );
    }
    Serial.println();
    fnElapsedStr( elapsedTimeString );
    Serial.printf( "%s - Startup Time : %d-%02d-%02d %02d:%02d:%02d\n", elapsedTimeString, (startTime.tm_year)+1900, (startTime.tm_mon)+1, startTime.tm_mday, startTime.tm_hour , startTime.tm_min, startTime.tm_sec );
    if( oldTickerValue != waitTime ) {
      tickerCam.detach( );
      tickerCam.attach( waitTime, funcCamTicker );
      oldTickerValue = waitTime;
    }
    if( tickerCamMissed > 1 ) {
      Serial.printf( "Missed %d tickers\n", tickerCamMissed - 1 );
    }
    tickerCamMissed = 0;
  }

  if( tickerBMEFired ) {

    tickerBMEFired = false;

    fnElapsedStr( elapsedTimeString );
#ifdef HAVE_BME280
    if( bme280Found ) {
      bmeSerialPrint();
    }
#else
    Serial.printf( "BME280 tick - %s\n", elapsedTimeString );
#endif

    if( tickerBMEMissed > 1 ) {
      Serial.printf( "Missed %d tickers\n", tickerBMEMissed - 1 );
    }
    tickerBMEMissed = 0;

  }

}
