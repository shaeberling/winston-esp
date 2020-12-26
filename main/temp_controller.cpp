#include "temp_controller.h"

#include <string>
#include <vector>

#include "esp_log.h"

#include "controller.h"
#include "htu21d_controller.h"

#ifdef __cplusplus
extern "C" {
#endif
extern uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif

static const char *TAG = "win-temp-ctrl";

TempController::TempController(HTU21DController* htu21d_controller)
    : htu21d_controller_(htu21d_controller) {
}

// override
std::vector<SensorConfig*> TempController::getSensors() const {
  std::vector<SensorConfig*> sensors;
  // TODO: This depends on whether another temp module was added or not.
  for (int i = 0; i < 2; ++i) {
    auto* c = new SensorConfig {
      .name = "temp",
      .idx = i,
      .update_interval_seconds = 10,
      .get_value = [this, i](void){ return std::to_string(this->getCelsius(i)); }
    };
    sensors.push_back(c);
  }

  auto* c = new SensorConfig {
    .name = "hum",
    .idx = 0,
    .update_interval_seconds = 10,
    .get_value = [this](void){ return std::to_string(this->getHumidity(0)); }
  };
  sensors.push_back(c);

  return sensors;
}

// public 
float TempController::getCelsius(int idx) const {
  // 0 is the internal temperature sensor inside the ESP32.
  if (idx == 0) {
    // Convert F -> C. Note: very inaccurate in an absolute sense.
    return (temprature_sens_read() - 32);
  } else if (htu21d_controller_ != NULL && idx == 1) {
    return htu21d_controller_->getCelsius();
  }

  // No other sensors are supported right now.
  ESP_LOGW(TAG, "Invalid temperature sensor index '%d'.", idx);
  return -1;
}

float TempController::getHumidity(int idx) const {
  if (htu21d_controller_ != NULL && idx == 0) {
    return htu21d_controller_->getHumidity();
  }
  ESP_LOGW(TAG, "Invalid humidity sensor index '%d'.", idx);
  return -1;
}
