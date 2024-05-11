
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

AsyncWebServer asyncWebServer(8080);
// AsyncWebSocket ws("/ws");

typedef struct {
        camera_fb_t * fb;
        size_t index;
} camera_frame_t;

extern bool timeLapse;

class AsyncBufferResponse: public AsyncAbstractResponse {
    private:
        uint8_t * _buf;
        size_t _len;
        size_t _index;
    public:
        AsyncBufferResponse(uint8_t * buf, size_t len, const char * contentType){
            _buf = buf;
            _len = len;
            _callback = nullptr;
            _code = 200;
            _contentLength = _len;
            _contentType = contentType;
            _index = 0;
        }
        ~AsyncBufferResponse(){
            if(_buf != nullptr){
                free(_buf);
            }
        }
        bool _sourceValid() const { return _buf != nullptr; }
        virtual size_t _fillBuffer(uint8_t *buf, size_t maxLen) override{
            size_t ret = _content(buf, maxLen, _index);
            if(ret != RESPONSE_TRY_AGAIN){
                _index += ret;
            }
            return ret;
        }
        size_t _content(uint8_t *buffer, size_t maxLen, size_t index){
            memcpy(buffer, _buf+index, maxLen);
            if((index+maxLen) == _len){
                free(_buf);
                _buf = nullptr;
            }
            return maxLen;
        }
};

class AsyncFrameResponse: public AsyncAbstractResponse {
    private:
        camera_fb_t * fb;
        size_t _index;
    public:
        AsyncFrameResponse(camera_fb_t * frame, const char * contentType){
            _callback = nullptr;
            _code = 200;
            _contentLength = frame->len;
            _contentType = contentType;
            _index = 0;
            fb = frame;
        }
        ~AsyncFrameResponse(){
            if(fb != nullptr){
                esp_camera_fb_return(fb);
            }
        }
        bool _sourceValid() const { return fb != nullptr; }
        virtual size_t _fillBuffer(uint8_t *buf, size_t maxLen) override{
            size_t ret = _content(buf, maxLen, _index);
            if(ret != RESPONSE_TRY_AGAIN){
                _index += ret;
            }
            return ret;
        }
        size_t _content(uint8_t *buffer, size_t maxLen, size_t index){
            memcpy(buffer, fb->buf+index, maxLen);
            if((index+maxLen) == fb->len){
                esp_camera_fb_return(fb);
                fb = nullptr;
            }
            return maxLen;
        }
};

class AsyncJpegStreamResponse: public AsyncAbstractResponse {
    private:
        camera_frame_t _frame;
        size_t _index;
        size_t _jpg_buf_len;
        uint8_t * _jpg_buf;
        uint64_t lastAsyncRequest;
    public:
        AsyncJpegStreamResponse(){
            _callback = nullptr;
            _code = 200;
            _contentLength = 0;
            _contentType = STREAM_CONTENT_TYPE;
            _sendContentLength = false;
            _chunked = true;
            _index = 0;
            _jpg_buf_len = 0;
            _jpg_buf = NULL;
            lastAsyncRequest = 0;
            memset(&_frame, 0, sizeof(camera_frame_t));
        }
        ~AsyncJpegStreamResponse(){
            if(_frame.fb){
                if(_frame.fb->format != PIXFORMAT_JPEG){
                    free(_jpg_buf);
                }
                esp_camera_fb_return(_frame.fb);
            }
        }
        bool _sourceValid() const {
            return true;
        }
        virtual size_t _fillBuffer(uint8_t *buf, size_t maxLen) override {
            size_t ret = _content(buf, maxLen, _index);
            if(ret != RESPONSE_TRY_AGAIN){
                _index += ret;
            }
            return ret;
        }
        size_t _content(uint8_t *buffer, size_t maxLen, size_t index){
            if(!_frame.fb || _frame.index == _jpg_buf_len){
                if(index && _frame.fb){
                    uint64_t end = (uint64_t)micros();
                    int fp = (end - lastAsyncRequest) / 1000;
                    // log_printf("Size: %uKB, Time: %ums (%.1ffps)\n", _jpg_buf_len/1024, fp); // TODO - implement logging
                    lastAsyncRequest = end;
                    if(_frame.fb->format != PIXFORMAT_JPEG){
                        free(_jpg_buf);
                    }
                    esp_camera_fb_return(_frame.fb);
                    _frame.fb = NULL;
                    _jpg_buf_len = 0;
                    _jpg_buf = NULL;
                }
                if(maxLen < (strlen(STREAM_BOUNDARY) + strlen(STREAM_PART) + strlen(JPG_CONTENT_TYPE) + 8)){
                    //log_w("Not enough space for headers");
                    return RESPONSE_TRY_AGAIN;
                }
                //get frame
                _frame.index = 0;

                _frame.fb = esp_camera_fb_get();
                if (_frame.fb == NULL) {
                    log_e("Camera frame failed");
                    return 0;
                }

                if(_frame.fb->format != PIXFORMAT_JPEG){
                    unsigned long st = millis();
                    bool jpeg_converted = frame2jpg(_frame.fb, 80, &_jpg_buf, &_jpg_buf_len);
                    if(!jpeg_converted){
                        log_e("JPEG compression failed");
                        esp_camera_fb_return(_frame.fb);
                        _frame.fb = NULL;
                        _jpg_buf_len = 0;
                        _jpg_buf = NULL;
                        return 0;
                    }
                    // log_i("JPEG: %lums, %uB", millis() - st, _jpg_buf_len);  // TODO - implement logging
                } else {
                    _jpg_buf_len = _frame.fb->len;
                    _jpg_buf = _frame.fb->buf;
                }

                //send boundary
                size_t blen = 0;
                if(index){
                    blen = strlen(STREAM_BOUNDARY);
                    memcpy(buffer, STREAM_BOUNDARY, blen);
                    buffer += blen;
                }
                //send header
                size_t hlen = sprintf((char *)buffer, STREAM_PART, JPG_CONTENT_TYPE, _jpg_buf_len);
                buffer += hlen;
                //send frame
                hlen = maxLen - hlen - blen;
                if(hlen > _jpg_buf_len){
                    maxLen -= hlen - _jpg_buf_len;
                    hlen = _jpg_buf_len;
                }
                memcpy(buffer, _jpg_buf, hlen);
                _frame.index += hlen;
                return maxLen;
            }

            size_t available = _jpg_buf_len - _frame.index;
            if(maxLen > available){
                maxLen = available;
            }
            memcpy(buffer, _jpg_buf+_frame.index, maxLen);
            _frame.index += maxLen;

            return maxLen;
        }
};

void asyncHandleRoot( AsyncWebServerRequest *request ) {

  // Serial.println( " asyncHandleRoot " );
  request->send( 200, "text/html", getHTMLRootText() );

}

void asyncHandleStatistics( AsyncWebServerRequest *request ) {

  // Serial.println( " asyncHandleStatistics " );
  request->send( 200, "text/html", getHTMLStatisticsText() );

}

void asyncHandleMetrics( AsyncWebServerRequest *request ) {

  // Serial.println( " asyncHandleMetrics " );
  request->send( 200, "text/plain", getMetricsText() );

}

void asyncHandleStatus( AsyncWebServerRequest *request ) {

  // Serial.println( " asyncHandleStatus " );
  AsyncWebServerResponse *response = request->beginResponse( 200, "application/json", getCameraStatus() );
  response->addHeader( "Access-Control-Allow-Origin", "*" );
  request->send( response );

}

void asyncHandleFullSetup( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/html", NOT_AUTHORIZED );
    return;
  }

  // Serial.println( " asyncHandleFullSetup " );

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
    AsyncWebParameter* arg = request->getParam( "user" );
    value = arg->value().c_str();
    if( value != http_username ) {
    }
  }
  if( request->hasParam( "password" ) ) {
    AsyncWebParameter* arg = request->getParam( "password" );
    value = arg->value().c_str();
    if( value != http_password ) {
    }
  }
  if( !request->authenticate( http_username, http_password ) )
    return request->requestAuthentication();  // Hm? Double-check this return ..
  request->send( 200, "text/html", LOGIN_SUCCESS );

}

void asyncHandleCapture( AsyncWebServerRequest *request ) {

  // Serial.println( " asyncHandleCapture " );
  // http://{CAM_IP}/capture?_cb=1701038417082
  String value = "";
  if( request->hasParam( "_cb" ) ) {
    AsyncWebParameter* arg = request->getParam( "_cb" );
    value = arg->value().c_str();
  }
  if( !timeLapse )  // if on timelapse mode just take last photo taken
    doSnapSavePhoto();
  AsyncWebServerResponse *response = request->beginResponse( 200, "image/jpeg", photoFrame );
  response->addHeader( "Content-Disposition", "inline; filename=photo.jpg" );
  response->addHeader( "Access-Control-Allow-Origin", "*" );
  response->addHeader( "X-Timestamp", value );
/* this has to be inside capture {}
  char ts[32];
  snprintf(ts, 32, "%lld.%06ld", fb->timestamp.tv_sec, fb->timestamp.tv_usec);
  response->addHeader( "X-Timestamp", (const char *)ts );
  */
  request->send( response );

}

void asyncHandleConnectPrusa( AsyncWebServerRequest *request ) {

  // WiFiClient prusa;

  // prusa.connect( ?.prusa.com, 443 );
  // prusa.println( ALL_HEADERS );
  // prusa.print( photoFrame );
  // prusa.println();
  // prusa.stop();

}

void asyncHandleWebSockets( AsyncWebServerRequest *request ) {

  // Serial.println( " asyncHandleWebSockets " );
  request->send( 200, "text/html", "Here Be WebSockets .." );

}

void asyncHandleStream( AsyncWebServerRequest *request ) {

  // Serial.println( " asyncHandleStream " );

  if ( timeLapse ) {  // FIXME: display a picture instead
    request->send( 200, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='6; URL=/setup'></head><body>Time Lapse Active!</body></html>" );
    return;
  }
  AsyncJpegStreamResponse *response = new AsyncJpegStreamResponse();
  response->addHeader( "Access-Control-Allow-Origin", "*" );
  request->send( response );

}

void asyncHandleCommand( AsyncWebServerRequest *request ) {

//  const query = `${baseHost}/control?var=${el.id}&val=${value}`
  String variable = "";
  String value = "";

  if( request->hasParam( "var" ) ) {
    AsyncWebParameter* arg = request->getParam( "var" );
    variable = arg->value().c_str();
    if( request->hasParam( "val" ) ) {
      AsyncWebParameter* arg = request->getParam( "val" );
      value = arg->value();
    }
  } else {
    request->send( 500, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='10; URL=/'></head><body>Wrong Input Parameter!</body></html>" );
    return;
  }

  Serial.printf( " asyncHandleCommand - %s - %s\n", variable, value );

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
#if defined( CAMERA_MODEL_AI_THINKER )
    flashEnabled = !flashEnabled;
#else
    flashEnabled = false;
#endif
  } else {
    err = -1;
  }

  if( err ) {
    request->send( 500, "text/html", "<!doctype html><html><head><meta http-equiv='refresh' content='10; URL=/'></head><body>Wrong Input Parameter!</body></html>" );
    return;
  }

  AsyncWebServerResponse *response = request->beginResponse( 200, "", "" );
  response->addHeader( "Access-Control-Allow-Origin", "*" );
  request->send( response );

}

void asyncHandleScan( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/html", NOT_AUTHORIZED );
    return;
  }

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

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/html", NOT_AUTHORIZED );
    return;
  }

  Serial.println( "Restarting in 5 seconds" );
  request->send( 200, "text/html", ESP_RESTART );
  delay( 2000 );
  WiFi.disconnect();
  ESP.restart();

}

void asyncHandleArchive( AsyncWebServerRequest *request ) {

  listDirectory( "/mozz-cam", request );

}

void asyncHandleSDCardRemount( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/html", NOT_AUTHORIZED );
    return;
  }

#ifdef HAVE_SDCARD
  SD_MMC.end();
  delay( 1000 );
  initSDCard();
  request->send( 200, "text/html", SD_CARD_REMOUNT );
#endif

}

void asyncHandleDelete( AsyncWebServerRequest *request ) {

  if( !request->authenticate( http_username, http_password ) ) {
    request->send( 200, "text/html", NOT_AUTHORIZED );
    return;
  }

  String webText;

  AsyncWebParameter* argDelete = request->getParam( "filename" );
  String fileName = argDelete->value();

  Serial.printf( " asyncHandleDelete - %s\n", fileName );
  webText = "ToDelete - ";
  webText += fileName;
  request->send( 200, "text/plain", webText );
  return;

//  deleteFile( SD_MMC, fileName );
  if( SD_MMC.remove( fileName.c_str() ) ) {
    webText = "Deleted - " + String( fileName );
    Serial.print( "Deleted - " );
    Serial.println( fileName );
  } else {
    webText = "Cannot delete - " + String( fileName );
    Serial.print( "Cannot delete - " );
    Serial.println( fileName );
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
    AsyncWebParameter* p = request->getParam( i );
    webText += String( p->name().c_str() ) + " : " + String( p->value().c_str() ) + "\r\n";
  }

  Serial.print( "Basename - " );
  Serial.print( fileName );
  Serial.print( " - " );
  Serial.println( webText );

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
  Serial.println( webText );

}

void initAsyncWebServer( void ) {

  asyncWebServer.on( "/", HTTP_GET, asyncHandleRoot );
  asyncWebServer.on( "/login", HTTP_POST, asyncHandleLogin );
  asyncWebServer.on( "/setup", HTTP_GET, asyncHandleFullSetup );
  asyncWebServer.on( "/stats", HTTP_GET, asyncHandleStatistics );

  asyncWebServer.on( "/fullsetup", HTTP_GET, asyncHandleFullSetup );
  asyncWebServer.on( "/status", HTTP_GET, asyncHandleStatus );

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

  asyncWebServer.on( "/prusa", HTTP_POST, asyncHandleConnectPrusa );  // TODO

  asyncWebServer.onNotFound( asyncHandleNotFound );

  asyncWebServer.begin();

}
