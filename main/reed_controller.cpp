#include "reed_controller.h"

#include "esp_log.h"

#include "driver/gpio.h"

static const char *TAG = "reed-controller";

ReedController::ReedController(const std::vector<int>& mapping) : mapping_(mapping) {
  for (int pin : mapping_) {
    initPin(pin);
  }
}

// public 
bool ReedController::is_closed(int idx) {
  if (mapping_.size() <= idx) {
    ESP_LOGE(TAG, "Illegal reed index '%d'", idx);
    return false;
  }
  int level = gpio_get_level( (gpio_num_t) mapping_[idx]);
  ESP_LOGI(TAG, "Reed controller status for %d --> %d", idx, level);
  return level == 0;
}

// private 
void ReedController::initPin(int n) {
  ESP_LOGI(TAG, "Initializing ping %d as REED.", n);
  gpio_config_t io_conf;
  // Interrupt of rising edge.
  io_conf.intr_type = GPIO_INTR_DISABLE;
  // Bit mask of the pins.
  io_conf.pin_bit_mask = 1ULL<<n;
  // Set as input mode.
  io_conf.mode = GPIO_MODE_INPUT;
  // Enable pull-up mode
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_config(&io_conf);
}

