
#include <Adafruit_BME280.h>    // DO *NOT* mix with #include "esp_camera.h" - sensor_t clash

#include "variables.h"

/// FIXME - double #define location - BEWARE of sensor_t
#define CAMERA_MODEL_AI_THINKER
// #define CAMERA_MODEL_XIAO_ESP32S3
/// FIXME - double #define location - BEWARE of sensor_t

#ifdef CAMERA_MODEL_AI_THINKER
 #define I2C_Freq 100000
 #define I2C_SDA 21
 #define I2C_SCL 22
 #define I2C_SDA2 13
 #define I2C_SCL2 12    // 16 fuqs-up camera PSRAM
#endif

Adafruit_BME280 bme;

void bmeSerialPrint( void ) {

  if( bme280Found ) {
#ifdef HAVE_BME280
    Serial.printf( "Temperature = %0.2f°C, ", bme.readTemperature() );  // \xB0 is °
    Serial.printf( "Pressure = %4.2f hPA, ", (bme.readPressure() / 100.0F) );
    Serial.printf( "Humidity = %.2f%%, ", bme.readHumidity() );
    Serial.printf( "Altitude = %.2f, ", bme.readAltitude( SEALEVELPRESSURE_HPA ) );
#endif
    Serial.printf( "%s\n", elapsedTimeString );
  }

}

void initBME( void ) {

#ifdef CAMERA_MODEL_AI_THINKER
  // Wire.begin( I2C_SDA, I2C_SCL );  // apparently not needed any more
  Wire1.begin( I2C_SDA2, I2C_SCL2 );
#endif

  bme280Found = true;
  // if( !bme.begin( 0x76 ) ) {
  if( !bme.begin( 0x76, &Wire1 ) ) {
    Serial.println( "Could not find a valid BME280 sensor, check wiring!" );
    bme280Found = false;
  }

}
