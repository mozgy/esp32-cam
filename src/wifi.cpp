
#include <ArduinoOTA.h>
#include "esp_camera.h"

#include "camera_model.h"
#include "mywifi.h"
#include "credentials.h"

/*
void waitForConnect( unsigned long timeout ) {

  unsigned long timeWiFi = millis();

  while( WiFi.status() != WL_CONNECTED ) {
    if( ( millis() - timeWiFi ) > timeout )
      break;
  }

}
  */

String get_wifi_status( int status ) {
  switch( status ) {
    case WL_IDLE_STATUS:
      return "WL_IDLE_STATUS";
    case WL_SCAN_COMPLETED:
      return "WL_SCAN_COMPLETED";
    case WL_NO_SSID_AVAIL:
      return "WL_NO_SSID_AVAIL";
    case WL_CONNECT_FAILED:
      return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST:
      return "WL_CONNECTION_LOST";
    case WL_CONNECTED:
      return "WL_CONNECTED";
    case WL_DISCONNECTED:
      return "WL_DISCONNECTED";
  }
  return "WL_NONE";
}

void WiFiStationConnected( WiFiEvent_t event, WiFiEventInfo_t info ) {

  log_i( "Connected to AP successfully!" );

}

void WiFiGotIP( WiFiEvent_t event, WiFiEventInfo_t info ) {

  log_i( "WiFi connected with IP address: %s", WiFi.localIP().toString().c_str() );
  // Serial.println( IPAddress( info.got_ip.ip_info.ip.addr ) );

}

void WiFiStationDisconnected( WiFiEvent_t event, WiFiEventInfo_t info ) {

  log_i( "Disconnected from WiFi access point" );
  log_i( "WiFi lost connection. Reason: %d", info.wifi_sta_disconnected.reason );
  log_i( "Reconnecting .." );
  // WiFi.disconnect( );
  // vTaskDelay( 4000 );
  WiFi.begin( ssid, password );

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
      log_d( "Stopping Camera" );
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

void initWiFi( void ) {

#ifdef CAMERA_MODEL_AI_THINKER
  flashLED( 300, true ); delay( 80 ); flashLED( 300, true );
#endif
  WiFi.softAPdisconnect( true );
//  WiFi.disconnect( true );
  WiFi.setMinSecurity( WIFI_AUTH_WPA_PSK );
//  esp_wifi_set_storage( WIFI_STORAGE_RAM );
//  // esp_wifi_set_storage( WIFI_STORAGE_FLASH );
//  uint32_t brown_reg_tmp = READ_PERI_REG( RTC_CNTL_BROWN_OUT_REG );
//  WRITE_PERI_REG( RTC_CNTL_BROWN_OUT_REG, 0 );
  WiFi.mode( WIFI_STA );
//  WRITE_PERI_REG( RTC_CNTL_BROWN_OUT_REG, brown_reg_tmp );
  WiFi.onEvent( WiFiStationConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED );
  WiFi.onEvent( WiFiGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP );
  WiFi.onEvent( WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED );
  WiFi.setHostname( ESP_CAM_HOSTNAME );
  WiFi.begin( ssid, password );
  WiFi.setSleep( false );

  WiFi.scanNetworks( true );
#ifdef CAMERA_MODEL_AI_THINKER
  flashLED( 50, true ); delay( 80 ); flashLED( 50, true ); delay( 80 ); flashLED( 50, true );
#endif

}
