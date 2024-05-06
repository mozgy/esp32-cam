#ifndef _MYWEBSERVER_H_
#define _MYWEBSERVER_H_

#include <WebServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *STREAM_PART = "Content-Type: %s\r\nContent-Length: %u\r\n\r\n";

static const char *JPG_CONTENT_TYPE = "image/jpeg";
static const char *BMP_CONTENT_TYPE = "image/x-windows-bmp";

static const char *NOT_AUTHORIZED = "<!doctype html><html><head><meta http-equiv='refresh' content='20; URL=/'><title>Cam</title><link rel='stylesheet' type='text/css' href='mozz.css'></head><body>Not Authorized!</body></html>";

extern AsyncWebServer asyncWebServer;

void asyncHandleRoot( AsyncWebServerRequest *request );
void asyncHandleStatistics( AsyncWebServerRequest *request );
void asyncHandleSetup( AsyncWebServerRequest *request );
void asyncHandleFullSetup( AsyncWebServerRequest *request );
void asyncHandleLogin( AsyncWebServerRequest *request );
void asyncHandleNotFound( AsyncWebServerRequest *request );
void listDirectory( String path, AsyncWebServerRequest *request );
esp_err_t loadFromSDCard( AsyncWebServerRequest *request );
void initAsyncWebServer( void );
void doSnapSavePhoto( void );

String getHTMLRootText( void );
String getHTMLStatisticsText( void );
String getHTMLSetupText( void );
String getHTMLFullSetupText( void );
String getMetricsText( void );
String getCameraStatus( void );

void fnElapsedStr( char *str );
void initSDCard( void );
extern bool SDCardOK;
extern int tickerCounter;

extern void reconfigureCamera( void );
extern void fnSetFrameSize( String frameSize );

#endif