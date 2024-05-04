
#include <SD_MMC.h>
#include <WiFi.h>
#include <Adafruit_BME280.h>    // DO *NOT* mix with #include "esp_camera.h" - sensor_t clash

#include "variables.h"

extern Adafruit_BME280 bme;
extern int tickerCamCounter;
extern size_t photoFrameLength;
extern bool SDCardOK;

String getMetricsText( void ) {

  String webText;

  webText = "# TYPE esp_cam_info counter\n";
  webText += "esp_cam_info " + String( tickerCamCounter ) + "\n";
  webText += "# TYPE esp_cam_info_id gauge\n";
  webText += "esp_cam_info_id " + String( CAM_SERIAL ) + "\n";
  webText += "# TYPE esp_cam_info_photo_len gauge\n";
  webText += "esp_cam_info_photo_len " + String( photoFrameLength ) + "\n";
  webText += "# TYPE esp_cam_info_rssi gauge\n";
  webText += "esp_cam_info_rssi " + String( WiFi.RSSI() ) + "\n";

#ifdef HAVE_SDCARD
  if ( SDCardOK ) {
    webText += "# TYPE esp_cam_info_sdcard_used gauge\n";
    webText += "esp_cam_info_sdcard_used " + String( SD_MMC.usedBytes() ) + "\n";
  }
#endif

#ifdef HAVE_BME280
  if ( bme280Found ) {
    webText += "# TYPE esp_cam_info_temperature_C gauge\n";
    webText += "esp_cam_info_temperature_C " + String( bme.readTemperature() ) + "\n";
    webText += "# TYPE esp_cam_info_humidity gauge\n";
    webText += "esp_cam_info_humidity " + String( bme.readHumidity() ) + "\n";
    webText += "# TYPE esp_cam_info_pressure gauge\n";
    webText += "esp_cam_info_pressure " + String( bme.readPressure() ) + "\n";
  }
#endif

  return webText;

}
