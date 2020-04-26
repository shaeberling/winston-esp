#include "temp_controller.h"

#include "esp_log.h"

#include "htu21d_controller.h"

#ifdef __cplusplus
extern "C" {
#endif
extern uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif

static const char *TAG = "temp-controller";

TempController::TempController(HTU21DController* htu21d_controller)
    : htu21d_controller_(htu21d_controller) {
}

// public 
float TempController::getCelsius(int idx) {
  // 0 is the internal temperature sensor inside the ESP32.
  if (idx == 0) {
    // Convert F -> C.
    return (temprature_sens_read() - 32);
  } else if (htu21d_controller_ != NULL && idx == 1) {
    return htu21d_controller_->getCelsius();
  }

  // No other sensors are supported right now.
  ESP_LOGW(TAG, "Invalid temperature sensor index '%d'.", idx);
  return -1;
}

float TempController::getHumidity(int idx) {
  if (htu21d_controller_ != NULL && idx == 0) {
    return htu21d_controller_->getHumidity();
  }
  ESP_LOGW(TAG, "Invalid humidity sensor index '%d'.", idx);
  return -1;
}
