#ifndef _MYWEBSERVER_H_
#define _MYWEBSERVER_H_

#include <WebServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

static const char *NOT_AUTHORIZED = "<!doctype html><html><head><meta http-equiv='refresh' content='20; URL=/'><title>Cam</title><link rel='stylesheet' type='text/css' href='mozz.css'></head><body>Not Authorized!</body></html>";
static const char *LOGIN_SUCCESS = "<!doctype html><html><head><meta http-equiv='refresh' content='6; URL=/'><title>Cam</title><link rel='stylesheet' type='text/css' href='mozz.css'></head><body>Login Success!</body></html>";
static const char *HTML_NOT_FOUND = "<!doctype html><html><head><meta http-equiv='refresh' content='40; URL=/'><title>Cam</title><link rel='stylesheet' type='text/css' href='mozz.css'></head><body>HTML File Not Found!</body></html>";
static const char *SD_CARD_REMOUNT = "<!doctype html><html><head><meta http-equiv='refresh' content='8; URL=/'><title>Cam</title><link rel='stylesheet' type='text/css' href='mozz.css'></head><body>SD Card Remount!</body></html>";
static const char *ESP_RESTART = "<!doctype html><html><head><meta http-equiv='refresh' content='20; URL=/'><title>Cam</title><link rel='stylesheet' type='text/css' href='mozz.css'></head><body>ESP32 Restart!</body></html>";
static const char *HTML_HEAD = "<!doctype html><html><head><title>Cam</title><link rel='stylesheet' type='text/css' href='mozz.css'></head>";

extern AsyncWebServer asyncWebServer;

void asyncHandleRoot( AsyncWebServerRequest* );
void asyncHandleStatistics( AsyncWebServerRequest* );
void asyncHandleSetup( AsyncWebServerRequest* );
void asyncHandleFullSetup( AsyncWebServerRequest* );
void asyncHandleLogin( AsyncWebServerRequest* );
void asyncHandleNotFound( AsyncWebServerRequest* );
void listDirectory( String, AsyncWebServerRequest* );
esp_err_t loadFromSDCard( AsyncWebServerRequest* );
void initAsyncWebServer( void );
void doSnapSavePhoto( void );
extern String photoSendPrusaConnect( void );

String getHTMLRootText( void );
String getHTMLStatisticsText( void );
String getHTMLSetupText( void );
String getHTMLFullSetupText( void );
String getMetricsText( void );
String getCameraStatus( void );

void fnElapsedStr( char* );
void initSDCard( void );
extern bool SDCardOK;
extern int tickerCounter;

extern void reconfigureCamera( void );
extern void fnSetFrameSize( String );

#endif
