#include "bme280_controller.h"

#include "controller.h"
#include "locking.h"

#include "bme280.h"

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"

#include <sstream>
#include <string>
#include <vector>

static const char* TAG = "bme280-ctrl";

BME280Controller::BME280Controller(const std::string& id,
                                   const gpio_num_t scl,
                                   const gpio_num_t sda,
                                   Locking* locking) :
    id_(id),
    gpio_scl_(scl),
    gpio_sda_(sda),
    locking_(locking),
    bme280_(0x76),
    initialized_(false) {
}

// override
bool BME280Controller::init() {
  if (!this->locking_->lockI2C(TAG)) {
    ESP_LOGE(TAG, "Cannot lock I2C bus access.");
    return false;
  }
  bool result = initInternal();
  if (!this->locking_->unlockI2C(TAG)) {
    ESP_LOGE(TAG, "Cannot unlock I2C bus access.");
    return false;
  }
  return result;
}

// private
bool BME280Controller::initInternal() {
  if (initialized_) {
    ESP_LOGW(TAG, "BME280 controller already initialized.");
    return false;
  }
  esp_err_t ret = bme280_.init(this->gpio_sda_, this->gpio_scl_);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure I2C for BME280.");
    return false;
  }
  ESP_LOGI(TAG, "Link to BME280 established.");

  bme280_.setDebug(true);

  initialized_ = true;
  return true;
}

// override
void BME280Controller::registerIO(std::vector<SensorConfig*>* sensors,
                                  std::vector<ActuatorConfig*>* actuators) {
  sensors->push_back(new SensorConfig {
    .name = "temp",
    .id = this->id_,
    .update_interval_seconds = 10,
    .get_value = [this](void){ return std::to_string(this->getCelsius()); }
  });

  sensors->push_back(new SensorConfig {
    .name = "hum",
    .id = this->id_,
    .update_interval_seconds = 10,
    .get_value = [this](void){ return std::to_string(this->getHumidity()); }
  });

  sensors->push_back(new SensorConfig {
    .name = "pressure",
    .id = this->id_,
    .update_interval_seconds = 10,
    .get_value = [this](void){ return std::to_string(this->getPressure()); }
  });
}


float BME280Controller::getCelsius() /* const */ {
  if (!initialized_) {
    this->init();
    ESP_LOGW(TAG, "BME280 controller not initialized.");
    return 0;
  }

  // Rate limit, since all three calls call the same method?
  bme280_reading_data result = getUpdateInternal();
  ESP_LOGI(TAG, "Temperature: %f", result.temperature);
  return result.temperature;
}

float BME280Controller::getHumidity() /* const */ {
  if (!initialized_) {
    ESP_LOGE(TAG, "BME280 controller not initialized.");
    return 0;
  }
  // Rate limit, since all three calls call the same method?
  bme280_reading_data result = getUpdateInternal();
  ESP_LOGI(TAG, "Humidity: %f", result.humidity);
  return result.humidity;
}

float BME280Controller::getPressure() /* const */ {
  if (!initialized_) {
    ESP_LOGE(TAG, "BME280 controller not initialized.");
    return 0;
  }
  // Rate limit, since all three calls call the same method?
  bme280_reading_data result = getUpdateInternal();
  ESP_LOGI(TAG, "Pressure: %f", result.pressure);
  return result.pressure;
}

// private
bme280_reading_data BME280Controller::getUpdateInternal() {
  bme280_reading_data result = {0, 0, 0};
  if (!this->locking_->lockI2C(TAG)) {
    ESP_LOGE(TAG, "Cannot lock I2C bus access.");
    return result;
  }
  result = bme280_.readSensorData();
  if (!this->locking_->unlockI2C(TAG)) {
    ESP_LOGE(TAG, "Cannot unlock I2C bus access.");
    return result;
  }
  return result;
}
