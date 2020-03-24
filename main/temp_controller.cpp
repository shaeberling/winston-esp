#include "temp_controller.h"

#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif
extern uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif

static const char *TAG = "temp-controller";

TempController::TempController() {
}

// public 
float TempController::get_celsius(int idx) {
  // 0 is the internal temperature sensor inside the ESP32.
  if (idx == 0) {
    // Convert F -> C.
    return (temprature_sens_read() - 32) / 1.8;
  }

  // No other sensors are supported right now.
  ESP_LOGW(TAG, "Invalid temperature sensor index '%d'.", idx);
  return -1;
}