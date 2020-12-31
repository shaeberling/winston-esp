#include "reed_controller.h"

#include "esp_log.h"

#include "driver/gpio.h"

static const char *TAG = "reed-controller";

ReedController::ReedController(const std::string& id, const gpio_num_t pin)
    : id_(id), pin_(pin) {
}

// override
bool ReedController::init() {
  ESP_LOGI(TAG, "Initializing pin %d as REED.", pin_);
  gpio_config_t io_conf;
  // No interrupt for now.
  // TODO: Add interrupt logic once we want to publish changes.
  io_conf.intr_type = GPIO_INTR_DISABLE;
  // Bit mask of the pins.
  io_conf.pin_bit_mask = 1ULL<<pin_;
  // Set as input mode.
  io_conf.mode = GPIO_MODE_INPUT;
  // Enable pull-up mode
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_config(&io_conf);
  return true;
}


// override
std::vector<SensorConfig*> ReedController::getSensors() {
  std::vector<SensorConfig*> sensors;

  auto* c = new SensorConfig {
    .name = "reed",
    .id = id_,
    .update_interval_seconds = 5,
    .get_value = [this](void){ return std::to_string(this->isClosed()); }
  };
  sensors.push_back(c);

  return sensors;
}

// public
bool ReedController::isClosed() {
  return gpio_get_level(pin_) == 1;
}

