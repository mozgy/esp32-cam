
#include <stddef.h>
#include <WiFiClientSecure.h>
#include <ESP32Ping.h>

#include "variables.h"
#include "credentials.h"
#include "certificate.h"

#define SEND_BLOCK_SIZE 1024
extern String photoFrameLength;

extern String prusaTokenStr;
extern String camFingerPrintStr;

bool prusaPrinterOnline;

String photoSendPrusaConnect( void ) {

  prusaPrinterOnline = true;
  bool itsalive = Ping.ping( PRUSA_PRINTER_IP, 2 );
  if( !itsalive ) {
    log_e( "Printer OFFline ?" );
    prusaPrinterOnline = false;
    return "Printer OFFline!";
  }

  prusaTokenStr = "" + String( prusa_token );
  camFingerPrintStr = "" + String( camera_fingerprint );

  WiFiClientSecure prusa;

  prusa.setCACert( root_CAs );
  prusa.setTimeout( 1000 );

  prusa.connect( "connect.prusa3d.com", 443 );
  prusa.println( "PUT https://connect.prusa3d.com/c/snapshot HTTP/1.1" );
  prusa.println( "Host: connect.prusa3d.com" );
  prusa.println( "User-Agent: ESP32-CAM Family" );
  prusa.println( "Connection: close" );

  // camFingerPrint += String( ESP.getEfuseMac() );

  prusa.println( "fingerprint: " + camFingerPrintStr );
  log_v( "fingerprint: %s", camFingerPrintStr.c_str() );
  prusa.println( "token: " + prusaTokenStr );
  log_v( "token: %s", prusaTokenStr.c_str() );
  prusa.println( "Content-Type: image/jpeg" );
  uint32_t photoFrameLength = photoFrame.length();
  prusa.println( "Content-Length: " + String( photoFrameLength ) );
  log_v( "Content-Length: %s", String( photoFrameLength ) );
  prusa.println( );

  char *photoFBPointer = &photoFrame[0];
  size_t loopNum = photoFrameLength / SEND_BLOCK_SIZE;
  while( loopNum > 0 ) {
    log_v( "FB dump loop number %d", loopNum );
    prusa.write( (const uint8_t*)photoFBPointer, SEND_BLOCK_SIZE );
    prusa.flush();
    photoFBPointer += SEND_BLOCK_SIZE;
    loopNum--;
  }
  size_t theRest = photoFrameLength % SEND_BLOCK_SIZE;
  if( theRest != 0 ) {      // corner case of photo length exactly multiple of SEND_BLOCK_SIZE
    prusa.write( (const uint8_t*)photoFBPointer, theRest );
  }
  prusa.println( "\r\n" );
  prusa.flush();

  String response = "";
  while( prusa.connected() ) {
    if( prusa.available() ) {
      response = prusa.readStringUntil( '\n' );
      log_v( "%s", response.c_str() );
    }
  }
  prusa.stop();

  return response;

}
