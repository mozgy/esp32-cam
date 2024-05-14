
#include <Adafruit_BME280.h>    // DO *NOT* mix with #include "esp_camera.h" - sensor_t clash

#include "camera_model.h"   // "esp_camera.h" and <Adafruit_BME280.h> - sensor_t clash
#include "variables.h"

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
    // \xB0 is °
    log_i( "Temperature = %0.2f°C, Pressure = %4.2f hPA, Humidity = %.2f%%, Altitude = %.2f", bme.readTemperature(), (bme.readPressure() / 100.0F), bme.readHumidity(), bme.readAltitude( SEALEVELPRESSURE_HPA ) );
#endif
    log_i( "%s", elapsedTimeString );
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
    log_e( "Could not find a valid BME280 sensor, check wiring!" );
    bme280Found = false;
  }

}
