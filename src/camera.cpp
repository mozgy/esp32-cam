
#include <SD_MMC.h>

#include "camera.h"
#include "camera_pins.h"

void initCam( void ) {

  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if( config.pixel_format == PIXFORMAT_JPEG ) {
    if( psramFound() ) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined( CAMERA_MODEL_ESP_EYE )
  pinMode( 13, INPUT_PULLUP );
  pinMode( 14, INPUT_PULLUP );
#endif

  // camera init
  esp_err_t err = esp_camera_init( &config );
  if ( err != ESP_OK ) {
    log_e( "Camera init failed with error 0x%x", err );
    delay( 2000 );
    ESP.restart();
  }
  log_i( "Camera ON!" );

  sensor_t * s = esp_camera_sensor_get();

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

  s->set_framesize( s, FRAMESIZE_VGA );

}

String getCameraStatus( void ) {

  String jsonResponse;
  sensor_t *sensor = esp_camera_sensor_get();

  jsonResponse = "{";
  jsonResponse += "\"framesize\":" + String( sensor->status.framesize );
  jsonResponse += ",\"quality\":" + String( sensor->status.quality );
  jsonResponse += ",\"brightness\":" + String( sensor->status.brightness );
  jsonResponse += ",\"contrast\":" + String( sensor->status.contrast );
  jsonResponse += ",\"saturation\":" + String( sensor->status.saturation );
  jsonResponse += ",\"sharpness\":" + String( sensor->status.sharpness );
  jsonResponse += ",\"special_effect\":" + String( sensor->status.special_effect );
  jsonResponse += ",\"wb_mode\":" + String( sensor->status.wb_mode );
  jsonResponse += ",\"awb\":" + String( sensor->status.awb );
  jsonResponse += ",\"awb_gain\":" + String( sensor->status.awb_gain );
  jsonResponse += ",\"aec\":" + String( sensor->status.aec );
  jsonResponse += ",\"aec2\":" + String( sensor->status.aec2 );
  jsonResponse += ",\"ae_level\":" + String( sensor->status.ae_level );
  jsonResponse += ",\"aec_value\":" + String( sensor->status.aec_value );
  jsonResponse += ",\"agc\":" + String( sensor->status.agc );
  jsonResponse += ",\"agc_gain\":" + String( sensor->status.agc_gain );
  jsonResponse += ",\"gainceiling\":" + String( sensor->status.gainceiling );
  jsonResponse += ",\"bpc\":" + String( sensor->status.bpc );
  jsonResponse += ",\"wpc\":" + String( sensor->status.wpc );
  jsonResponse += ",\"raw_gma\":" + String( sensor->status.raw_gma );
  jsonResponse += ",\"lenc\":" + String( sensor->status.lenc );
  jsonResponse += ",\"dcw\":" + String( sensor->status.dcw );
  jsonResponse += ",\"colorbar\":" + String( sensor->status.colorbar );
  jsonResponse += ",\"vflip\":" + String( sensor->status.vflip );
  jsonResponse += ",\"hmirror\":" + String( sensor->status.hmirror );
//  jsonResponse += ",\"pixformat\":" + String( sensor->pixformat );
//  jsonResponse += ",\"xclk\":" + String( sensor->xclk_freq_hz / 1000000 );
  jsonResponse += ",\"timelapse\":";
  jsonResponse += ( timeLapse ) ? "1" : "0";
  jsonResponse += ",\"flashled\":";
  jsonResponse += ( flashEnabled ) ? "1" : "0";
  jsonResponse += ",\"rotation\":" + String( imageRotation );
#ifdef PRUSA_CONNECT
  jsonResponse += ",\"prusaconnect\":";
  jsonResponse += ( prusaConnectActive ) ? "1" : "0";
#endif
  jsonResponse += "}";

  log_d( "%s", jsonResponse.c_str() );
  return jsonResponse;

}

void flashON( void ) {

// global settings - ignoring html on/off
//  if( !FLASH_ENABLE )
//    return;

// html switch on/off
  if( !flashEnabled )
    return;

#ifdef CAMERA_MODEL_AI_THINKER
  pinMode( FLASH_LED, OUTPUT );
  digitalWrite( FLASH_LED, HIGH );
#endif
#ifdef CAMERA_MODEL_ESP32S3_CAM
  rgbLedWrite( FLASH_LED, RGB_BRIGHTNESS, RGB_BRIGHTNESS, RGB_BRIGHTNESS );
#endif
  log_d( "Flash is ON, smile!" );

}

void flashON( bool forcedFlash ) {

  if( !forcedFlash )
    return;

#ifdef CAMERA_MODEL_AI_THINKER
  pinMode( FLASH_LED, OUTPUT );
  digitalWrite( FLASH_LED, HIGH );
#endif
#ifdef CAMERA_MODEL_ESP32S3_CAM
  rgbLedWrite( FLASH_LED, RGB_BRIGHTNESS, RGB_BRIGHTNESS, RGB_BRIGHTNESS );
#endif
  log_d( "Flash is ON, smile!" );

}

void flashON( uint8_t R, uint8_t G, uint8_t B ) {

#ifdef CAMERA_MODEL_ESP32S3_CAM
  rgbLedWrite( FLASH_LED, R, G, B );
#endif

}

void flashOFF( void ) {

// global settings - ignoring html on/off
//  if( !FLASH_ENABLE )
//    return;

// html switch on/off
  if( !flashEnabled )
    return;

#ifdef CAMERA_MODEL_AI_THINKER
  pinMode( FLASH_LED, OUTPUT );
  digitalWrite( FLASH_LED, LOW );
#endif
#ifdef CAMERA_MODEL_ESP32S3_CAM
  rgbLedWrite( FLASH_LED, 0, 0, 0 );
#endif

// DATA1 / Flash LED - PIN4
// turn off AI-Thinker Board Flash LED
// FIXME - findout if pinMode OUTPUT makes any problems here
//  // rtc_gpio_hold_en( GPIO_NUM_4 );
//  pinMode( FLASH_LED, OUTPUT );
//  digitalWrite( FLASH_LED, LOW );

}

void flashOFF( bool forcedFlash ) {

  if( !forcedFlash )
    return;

#ifdef CAMERA_MODEL_AI_THINKER
  pinMode( FLASH_LED, OUTPUT );
  digitalWrite( FLASH_LED, LOW );
#endif
#ifdef CAMERA_MODEL_ESP32S3_CAM
  rgbLedWrite( FLASH_LED, 0, 0, 0 );
#endif

}

void flashLED( uint32_t flashONTime ) {

// html switch on/off
  if( !flashEnabled )
    return;

  flashON();
  delay( flashONTime );
  flashOFF();

}

void flashLED( uint32_t flashONTime, bool forcedFlash ) {

  if( !forcedFlash )
    return;

  flashON( true );
  delay( flashONTime );
  flashOFF( true );

}

// CAVEAT - capture *will* fail if stream is active - FIXME!
void doSnapSavePhoto( void ) {

  File photoFP;
  String photoFileDir;
  String photoFileName;
  camera_fb_t * photoFrameBuffer = NULL;
  photoSnapTime.tm_year = 0;
  photoFrame = "";

  getLocalTime( &photoSnapTime, 5000 );

  if( timeLapse && SDCardOK ) {
    String photoFileDir;
    String photoFileName;

    photoFileDir = String( "/mozz-cam/" );
    if( !SD_MMC.mkdir( photoFileDir ) ) {
      // log_e( "MKDIR Failed!" ); // dir could exist
    }
    sprintf( currentDateTime, "%04d\0", (photoSnapTime.tm_year)+1900 );
    photoFileDir += String( currentDateTime );  // /mozz-cam/YYYY
    if( !SD_MMC.mkdir( photoFileDir ) ) {
      // log_e( "MKDIR Failed!" ); // dir could exist
    }
    sprintf( currentDateTime, "/%02d%02d\0", (photoSnapTime.tm_mon)+1, photoSnapTime.tm_mday );
    photoFileDir += String( currentDateTime );  // /mozz-cam/YYYY/MMDD
    if( !SD_MMC.mkdir( photoFileDir ) ) {
      // log_e( "MKDIR Failed!" ); // dir could exist
    }
    sprintf( currentDateTime, "/%02d\0", photoSnapTime.tm_hour );
    photoFileDir += String( currentDateTime );
    if( !SD_MMC.mkdir( photoFileDir ) ) {
      // log_e( "MKDIR Failed!" ); // dir could exist
    }

    // yes, I know it can be oneliner -
    sprintf( currentDateTime, "%04d", (photoSnapTime.tm_year)+1900 );
    sprintf( currentDateTime, "%s%02d", currentDateTime, (photoSnapTime.tm_mon)+1 );
    sprintf( currentDateTime, "%s%02d", currentDateTime, photoSnapTime.tm_mday );
    sprintf( currentDateTime, "%s%02d", currentDateTime, photoSnapTime.tm_hour );
    sprintf( currentDateTime, "%s%02d", currentDateTime, photoSnapTime.tm_min );
    sprintf( currentDateTime, "%s%02d\0", currentDateTime, photoSnapTime.tm_sec );
    photoFileName = photoFileDir + String( "/photo-" ) + currentDateTime + String( ".jpg" ) ;
    log_d( "%s", photoFileName.c_str() );

    photoFP = SD_MMC.open( photoFileName, FILE_WRITE );
    if( !photoFP ) {
      log_e( "SD Card file open for write error" );
      return;
    }
  }

  flashON();
  if( flashEnabled )
    delay( 200 );
    // vTaskDelay(150 / portTICK_PERIOD_MS);
    // The LED needs to be turned on ~150ms before the call to esp_camera_fb_get()
    // or it won't be visible in the frame. A better way to do this is needed.

  int64_t capture_start = esp_timer_get_time();
  photoFrameBuffer = esp_camera_fb_get();
//  if( !photoFrameBuffer )
//    photoFrameBuffer = esp_camera_fb_get(); // second time the charm ??
  flashOFF();
  if( !photoFrameBuffer ) {
    log_e( "Camera Capture Failed" );
    // photoFrame = load_from_dataFS( "no-pic-200x200.png" );
    return;
  }
  // if (fb->format == PIXFORMAT_JPEG) // ToDo Check, JPEG mandatory

  photoFrameLength = photoFrameBuffer->len;
  // log_v( "Picture length : %d", photoFrameLength );

  if( imageRotation == 1 || timeLapse ) {    // 1 == top-left aka no rotation
    photoFrame += String( (char *) photoFrameBuffer->buf, photoFrameLength );
  } else {
    // insert Exif data here
    photoFrame += '\xff';
    photoFrame += '\xd8';
    photoFrame += '\xff';
    photoFrame += '\xe1';
    photoFrame += '\x00';
    photoFrame += '\x26';
    photoFrame += "Exif";
    photoFrame += '\x00';
    photoFrame += '\x00';
    photoFrame += '\x49';
    photoFrame += '\x49';
    photoFrame += '\x2a';
    photoFrame += '\x00';
    photoFrame += '\x08';
    photoFrame += '\x00';
    photoFrame += '\x00';
    photoFrame += '\x00';
    photoFrame += '\x01';
    photoFrame += '\x00';
    photoFrame += '\x12';
    photoFrame += '\x01';
    photoFrame += '\x03';
    photoFrame += '\x00';
    photoFrame += '\x01';
    photoFrame += '\x00';
    photoFrame += '\x00';
    photoFrame += '\x00';
    photoFrame += (char)imageRotation;
    photoFrame += '\x00';
    photoFrame += '\x00';
    photoFrame += '\x00';
    photoFrame += '\x00';
    photoFrame += '\x00';
    photoFrame += '\x00';
    photoFrame += '\x00';
    photoFrame += '\x00';
    photoFrame += '\x00';
    photoFrame += '\x00';
    photoFrame += '\x00';
/*
  '\xff', '\xd8',                   // JPEG SOI
  '\xff', '\xe1',                   // Exif APP1
  '\x00', '\x26',                   // length
  'Exif', '\x00', '\x00',           // Exif header
  '\x49', '\x49', '\x2a', '\x00',   // TIFF header
  '\x08', '\x00', '\x00', '\x00',
  '\x01', '\x00',                   // only one IFD0 entry
  '\x12', '\x01',                   // TagTiffOrientation 0x0112
  '\x03', '\x00',
  '\x01', '\x00', '\x00', '\x00',
  (uint8_t imageRotation), '\x00', '\x00', '\x00',   // 1 - 0, 3 - 180, 6 - 90, 8 - 270
  '\x00', '\x00', '\x00', '\x00',   // end of IFD0 marker
  '\x00', '\x00', '\x00', '\x00'    // end of Exif data marker
  */
    photoFrame += String( ( (char *) photoFrameBuffer->buf ) + 2, photoFrameLength - 2 );
    photoFrameLength += 38;         // TODO - put all above into nice struct so length can be compile time calculated
  }

  //  //replace this with your own function
  //  process_image(fb->width, fb->height, fb->format, fb->buf, fb->len);
  if( timeLapse && SDCardOK ) {
    photoFP.write( photoFrameBuffer->buf, photoFrameLength );
    photoFP.close();
    log_d( "Time Lapse ON - Wrote File - Used space: %lluMB", SD_MMC.usedBytes() / (1024 * 1024) );
  }

  //return the frame buffer back to the driver for reuse
  esp_camera_fb_return( photoFrameBuffer );
  photoFrameBuffer = NULL;

  int64_t capture_end = esp_timer_get_time();
  log_d( "Capture Time: %uB %ums", (uint32_t)( photoFrameLength ), (uint32_t)( ( capture_end - capture_start )/1000 ) );
//  log_d( "Total space: %lluMB, Used space: %lluMB", SD_MMC.totalBytes() / (1024 * 1024), SD_MMC.usedBytes() / (1024 * 1024) );

}

/*
typedef struct {
    framesize_t framesize;//0 - 10
    bool scale;
    bool binning;
    uint8_t quality;//0 - 63
    int8_t brightness;//-2 - 2
    int8_t contrast;//-2 - 2
    int8_t saturation;//-2 - 2
    int8_t sharpness;//-2 - 2
    uint8_t denoise;
    uint8_t special_effect;//0 - 6
    uint8_t wb_mode;//0 - 4
    uint8_t awb;
    uint8_t awb_gain;
    uint8_t aec;
    uint8_t aec2;
    int8_t ae_level;//-2 - 2
    uint16_t aec_value;//0 - 1200
    uint8_t agc;
    uint8_t agc_gain;//0 - 30
    uint8_t gainceiling;//0 - 6
    uint8_t bpc;
    uint8_t wpc;
    uint8_t raw_gma;
    uint8_t lenc;
    uint8_t hmirror;
    uint8_t vflip;
    uint8_t dcw;
    uint8_t colorbar;
} camera_status_t;
  */