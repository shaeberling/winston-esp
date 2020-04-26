#include "hall_effect_controller.h"

#include "esp_log.h"
#include <driver/adc.h>

static const char *TAG = "hall-controller";

HallEffectController::HallEffectController() : initialized_(false) {
}

// public 
void HallEffectController::init() {
  // "Reading from it uses channels 0 and 3 of ADC1 (GPIO 36 and 39).""
  // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html
  adc1_config_width(ADC_WIDTH_BIT_12);
  initialized_ = true;
}


// public 
int HallEffectController::getValue(int idx) {
  if (!initialized_) {
    ESP_LOGW(TAG, "Hall effect controller not initialized!");
    return 0;
  }

  // 0 is the internal hall effect sensor inside the ESP32.
  if (idx == 0) {
    return hall_sensor_read();
  }

  // No other sensors are supported right now.
  ESP_LOGW(TAG, "Invalid hall effect sensor index '%d'.", idx);
  return -0;
}
