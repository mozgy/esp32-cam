
#include <stddef.h>
#include <WebServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <LittleFS.h>
#include <SD_MMC.h>
#include <base64.h>

#include "variables.h"
#include "mywebserver.h"
#include "credentials.h"
#include "camera.h"
#include "asyncResponseClass.h"

AsyncWebServer asyncWebServer(8080);
// AsyncWebSocket ws("/ws");

extern bool timeLapse;
// AsyncJpegStreamResponse *streamActive;

bool checkWebAuth( AsyncWebServerRequest *request ) {

  if( !request->authenticate( httpUsernameStr.c_str(), httpPasswordStr.c_str() ) ) {
    request->send( 200, "text/html", NOT_AUTHORIZED );
    log_d("auth-false");
    return false;
  }
  log_d("auth-true");
  return true;

}

void asyncHandleRoot( AsyncWebServerRequest *request ) {

  log_d( " asyncHandleRoot " );
  request->send( 200, "text/html", getHTMLRootText() );

}

void asyncHandleStatistics( AsyncWebServerRequest *request ) {

  log_d( " asyncHandleStatistics " );
  request->send( 200, "text/html", getHTMLStatisticsText() );

}

void asyncHandleMetrics( AsyncWebServerRequest *request ) {

  log_d( " asyncHandleMetrics " );
  request->send( 200, "text/plain", getMetricsText() );

}

void asyncHandleFileConfig( AsyncWebServerRequest *request ) {

  log_d( " asyncHandleFileConfig " );
  request->send( 200, "text/html", NOT_AUTHORIZED ); // reading config file always forbidden through web!

}

void asyncHandleStatus( AsyncWebServerRequest *request ) {

  log_d( " asyncHandleStatus " );
  AsyncWebServerResponse *response = request->beginResponse( 200, "application/json", getCameraStatus() );
  response->addHeader( "Access-Control-Allow-Origin", "*" );
  request->send( response );

}

void asyncHandleFullSetup( AsyncWebServerRequest *request ) {

  if( !checkWebAuth( request ) )
    return;

  log_d( " asyncHandleFullSetup " );

  String fileName = "/esp32setup.html";

  // request->send( LittleFS.open( fileName.c_str() ), fileName.c_str(), "text/html" );

  File filePointer = LittleFS.open( fileName.c_str(), "r" );
  if( filePointer ) {
    if( filePointer.available() ) {
      String stringHTMLFullSetupText = filePointer.readString();
      request->send( 200, "text/html", stringHTMLFullSetupText );
    }
    filePointer.close();
    return;
  }
  request->send( 200, "text/html", HTML_NOT_FOUND );

}

void asyncHandleLogin( AsyncWebServerRequest *request ) {

  String value = "";
  if( request->hasParam( "user" ) ) {
    const AsyncWebParameter* arg = request->getParam( "user" );
    value = arg->value().c_str();
    if( value != httpUsernameStr ) {
    }
  }
  if( request->hasParam( "password" ) ) {
    const AsyncWebParameter* arg = request->getParam( "password" );
    value = arg->value().c_str();
    if( value != httpPasswordStr ) {
    }
  }
  if( !request->authenticate( httpUsernameStr.c_str(), httpPasswordStr.c_str() ) )
    return request->requestAuthentication();  // Hm? Double-check this return ..
  request->send( 200, "text/html", LOGIN_SUCCESS );

}

void asyncHandleCapture( AsyncWebServerRequest *request ) {

  log_d( " asyncHandleCapture " );
  // http://{CAM_IP}/capture?_cb=1701038417082
  String value = "";
  if( request->hasParam( "_cb" ) ) {
    const AsyncWebParameter* arg = request->getParam( "_cb" );
    value = arg->value().c_str();
  }
  if( !timeLapse )  // if on timelapse mode just take last photo taken
    doSnapSavePhoto();
  AsyncWebServerResponse *response = request->beginResponse( 200, "image/jpeg", (const u_int8_t *)photoFrame.c_str(), photoFrameLength );
  response->addHeader( "Content-Disposition", "inline; filename=photo.jpg" );
  response->addHeader( "Access-Control-Allow-Origin", "*" );
  response->addHeader( "X-Timestamp", value );
/* this has to be inside capture {} or _not_
  char ts[32];
  snprintf(ts, 32, "%lld.%06ld", fb->timestamp.tv_sec, fb->timestamp.tv_usec);
  response->addHeader( "X-Timestamp", (const char *)ts );
  */
  request->send( response );

}

void asyncHandlePrusaConnect( AsyncWebServerRequest *request ) {

  log_d( " asyncHandleConnectPrusa " );

  String response;
  response = photoSendPrusaConnect();
  request->send( 200, "text/plain", response );

}

void asyncHandleWebSockets( AsyncWebServerRequest *request ) {

  log_d( " asyncHandleWebSockets " );
  request->send( 200, "text/html", "Here Be WebSockets .." );

}

void asyncHandleStream( AsyncWebServerRequest *request ) {

  if( !checkWebAuth( request ) ) {
    request->send( LittleFS, "/NotAuth.jpg", "image/jpg" );
    return;
  }

  log_d( " asyncHandleStream " );

#ifdef CAMERA_MODEL_ESP32S3_CAM
  neopixelWrite( FLASH_LED, 12, 0, 0 ); // turn ON faint RED :)
#endif

  if ( timeLapse ) {
    // request->send( 200, "text/html", TIME_LAPSE_ACTIVE );
    request->send( LittleFS, "/TLActive.jpg", "image/jpg" );
    return;
  }
  AsyncJpegStreamResponse *response = new AsyncJpegStreamResponse();
//  streamActive = response;
  response->addHeader( "Access-Control-Allow-Origin", "*" );
  request->send( response );

}

void asyncHandleCommand( AsyncWebServerRequest *request ) {

//  const query = `${baseHost}/control?var=${el.id}&val=${value}`
  String variable = "";
  String value = "";

  if( request->hasParam( "var" ) ) {
    const AsyncWebParameter* arg = request->getParam( "var" );
    variable = arg->value().c_str();
    if( request->hasParam( "val" ) ) {
      const AsyncWebParameter* arg = request->getParam( "val" );
      value = arg->value();
    }
  } else {
    request->send( 500, "text/html", WRONG_INPUT );
    return;
  }

  log_d( " asyncHandleCommand - %s - %s", variable, value );

  sensor_t *sensor = esp_camera_sensor_get();
  int err = 0;
  int valueNum = value.toInt();

  if( variable == "framesize" ) {
    if( sensor->pixformat == PIXFORMAT_JPEG ) {
      err = sensor->set_framesize( sensor, (framesize_t)valueNum );
    }
  } else if( variable == "quality" ) {
    err = sensor->set_quality( sensor, valueNum );
  } else if( variable == "brightness" ) {
    err = sensor->set_brightness( sensor, valueNum );
  } else if( variable == "contrast" ) {
    err = sensor->set_contrast( sensor, valueNum );
  } else if( variable == "saturation" ) {
    err = sensor->set_saturation( sensor, valueNum );
  } else if( variable == "sharpness" ) {
    err = sensor->set_sharpness( sensor, valueNum );
  } else if( variable == "special_effect" ) {
    err = sensor->set_special_effect( sensor, valueNum );
  } else if( variable == "wb_mode" ) {
    err = sensor->set_wb_mode( sensor, valueNum );
  } else if( variable == "awb" ) {
    err = sensor->set_whitebal( sensor, valueNum );
  } else if( variable == "vflip" ) {
    err = sensor->set_vflip( sensor, valueNum );
  } else if( variable == "hmirror" ) {
    err = sensor->set_hmirror( sensor, valueNum );
  } else if( variable == "timelapse" ) {
    timeLapse = !timeLapse;
  } else if( variable == "flashled" ) {
#if defined(CAMERA_MODEL_AI_THINKER) || defined (CAMERA_MODEL_ESP32S3_CAM)
    flashEnabled = !flashEnabled;
#else
    flashEnabled = false;
#endif
  } else if( variable == "rotation" ) {
    imageRotation = valueNum;
  } else if( variable == "prusaconnect" ) {
#if defined(PRUSA_CONNECT)
    prusaConnectActive = !prusaConnectActive;
#else
    prusaConnectActive = false;
#endif
  } else {
    err = -1;
  }

  if( err ) {
    request->send( 500, "text/html", WRONG_INPUT ); // ToDo: add css
    return;
  }

  AsyncWebServerResponse *response = request->beginResponse( 200, "", "" );
  response->addHeader( "Access-Control-Allow-Origin", "*" );
  request->send( response );

}

void asyncHandleScan( AsyncWebServerRequest *request ) {

  if( !checkWebAuth( request ) )
    return;

//First request will return 0 results unless you start scan from somewhere else (loop/setup)
//Do not request more often than 3-5 seconds
  String json = "[";
  int n = WiFi.scanComplete();
  if( n == -2 ) {
    WiFi.scanNetworks( true );
  } else if( n ) {
    for( int i = 0; i < n; ++i ) {
      if( i ) json += ",";
      json += "{";
      json += "\"rssi\":"+String(WiFi.RSSI(i));
      json += ",\"ssid\":\""+WiFi.SSID(i)+"\"";
      json += ",\"bssid\":\""+WiFi.BSSIDstr(i)+"\"";
      json += ",\"channel\":"+String(WiFi.channel(i));
      json += ",\"secure\":"+String(WiFi.encryptionType(i));
//      json += ",\"hidden\":"+String(WiFi.isHidden(i)?"true":"false");
      json += "}";
    }
    WiFi.scanDelete();
    if( WiFi.scanComplete() == -2 ){
      WiFi.scanNetworks( true );
    }
  }
  json += "]";
  request->send( 200, "application/json", json );
  json = String();

}

void asyncHandleESPReset( AsyncWebServerRequest *request ) {

  if( !checkWebAuth( request ) )
    return;

  log_v( "Restarting in 5 seconds" );
  request->send( 200, "text/html", ESP_RESTART );
  delay( 3000 );
  // WiFi.disconnect();
  // delay( 1000 );
  ESP.restart();

}

void asyncHandleArchive( AsyncWebServerRequest *request ) {

  if( !checkWebAuth( request ) )
    return;

  listDirectory( "/mozz-cam", request );

}

void asyncHandleSDCardRemount( AsyncWebServerRequest *request ) {

  if( !checkWebAuth( request ) )
    return;

  log_d( " asyncHandleSDCardRemount " );

#ifdef HAVE_SDCARD
  SD_MMC.end();
  delay( 1000 );
  initSDCard();
  // FOOD_FOR_THOUGHT - load config here ??
  request->send( 200, "text/html", SD_CARD_REMOUNT );
#endif

}

void asyncHandleDelete( AsyncWebServerRequest *request ) {

  if( !checkWebAuth( request ) )
    return;

  String webText;

  const AsyncWebParameter* argDelete = request->getParam( "filename" );
  String fileName = argDelete->value();

  log_d( " asyncHandleDelete - %s\n", fileName );
  webText = "ToDelete - ";
  webText += fileName;
  request->send( 200, "text/plain", webText );
  return;   // Gate closed - need to retest spam load on SDCard

//  deleteFile( SD_MMC, fileName );
  if( SD_MMC.remove( fileName.c_str() ) ) {
    webText = "Deleted - " + String( fileName );
    log_v( "Deleted - %s", fileName.c_str() );
  } else {
    webText = "Cannot delete - " + String( fileName );
    log_v( "Cannot delete - %s", fileName.c_str() );
  }
  request->send( 200, "text/plain", webText );

}

void asyncHandleNotFound( AsyncWebServerRequest *request ) {

  String path = request->url();
  String dataType = "text/plain";
  String webText;

  int lastSlash = path.lastIndexOf( '/' );
  String fileName = path.substring( lastSlash, path.length() );

  webText = "URI: ";
  webText += request->url();
  webText += " - Method: ";
  webText += ( request->method() == HTTP_GET ) ? "GET" : "POST";
  webText += ", Parameters: ";
  webText += request->params();
  webText += "\n";
  for( uint8_t i = 0 ; i < request->params(); i++ ) {
    const AsyncWebParameter* p = request->getParam( i );
    webText += String( p->name().c_str() ) + " : " + String( p->value().c_str() ) + "\r\n";
  }

  log_v( "Basename - %s - %s", fileName.c_str(), webText.c_str() );

  bool fileLocalFS = false;
  if( fileName.endsWith( ".css" ) ) {
    dataType = "text/css";
    fileLocalFS = true;
  } else if( fileName.endsWith( ".html" ) ) {
    dataType = "text/html";
    fileLocalFS = true;
  } else if( fileName.endsWith( ".ico" ) ) {
    dataType = "image/png";
    fileLocalFS = true;
  } else if( fileName.endsWith( ".js" ) ) {
    dataType = "aplication/javascript";
    fileLocalFS = true;
  }
  if( fileLocalFS ) {
    request->send( LittleFS.open( fileName.c_str() ), fileName.c_str(), dataType );
    // File fp = LittleFS.open( fileName.c_str() ); // FIXME - NO WORKY
    // request->send( fp, fileName.c_str(), dataType );
    // fp.close();
    return;
  } else {
    if( loadFromSDCard( request ) == ESP_OK ) {
      return;
    }
  }

  webText = "\nNo Handler\r\n" + webText;
  request->send( 404, "text/plain", webText );
  log_v( "%s", webText.c_str() );

}

void initAsyncWebServer( void ) {

  asyncWebServer.on( "/", HTTP_GET, asyncHandleRoot );
  asyncWebServer.on( "/login", HTTP_POST, asyncHandleLogin );
  asyncWebServer.on( "/setup", HTTP_GET, asyncHandleFullSetup );
  asyncWebServer.on( "/stats", HTTP_GET, asyncHandleStatistics );

  asyncWebServer.on( "/fullsetup", HTTP_GET, asyncHandleFullSetup );
  asyncWebServer.on( "/status", HTTP_GET, asyncHandleStatus );
  asyncWebServer.on( "/config", HTTP_GET, asyncHandleFileConfig );

  asyncWebServer.on( "/control", HTTP_POST, asyncHandleCommand );
  asyncWebServer.on( "/capture", HTTP_GET, asyncHandleCapture );
  asyncWebServer.on( "/stream", HTTP_GET, asyncHandleStream );
  asyncWebServer.on( "/ws", HTTP_GET, asyncHandleWebSockets );  // TODO

  asyncWebServer.on( "/delete", HTTP_GET, asyncHandleDelete );  // TODO
  asyncWebServer.on( "/archive", HTTP_GET, asyncHandleArchive );
  asyncWebServer.on( "/sdcard", HTTP_GET, asyncHandleSDCardRemount );

  asyncWebServer.on( "/scan", HTTP_GET, asyncHandleScan );
  asyncWebServer.on( "/espReset", HTTP_GET, asyncHandleESPReset );

  asyncWebServer.on( "/metrics", HTTP_GET, asyncHandleMetrics );

  asyncWebServer.on( "/prusa", HTTP_POST, asyncHandlePrusaConnect );

  asyncWebServer.onNotFound( asyncHandleNotFound );

  asyncWebServer.begin();

}
