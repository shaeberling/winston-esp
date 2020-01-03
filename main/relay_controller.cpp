#include "relay_controller.h"

#include "esp_log.h"

#include "driver/gpio.h"

static const char *TAG = "relay-controller";

RelayController::RelayController(const std::vector<int>& mapping) : mapping_(mapping) {
  for (int pin : mapping_) {
    initPin(pin);
  }
}

// public 
void RelayController::switch_on(int idx, bool on) {
  if (mapping_.size() <= idx) {
    ESP_LOGW(TAG, "Illegal relay index '%d'", idx);
    return;
  }
  ESP_LOGI(TAG, "Swtching relay %d %s", idx, (on ? "ON" : "OFF"));
  // For relays that are switched by pulling them down.
  gpio_set_level((gpio_num_t) mapping_[idx], (on ? 0 : 1));
}

// private 
void RelayController::initPin(int n) {
  ESP_LOGI(TAG, "Initializing ping %d as RELAY.", n);
  gpio_config_t io_conf;
  // No interrupt.
  io_conf.intr_type = GPIO_INTR_DISABLE;
  // Bit mask of the pins.
  io_conf.pin_bit_mask = 1ULL<<n;
  // Set as output mode.
  io_conf.mode = GPIO_MODE_OUTPUT;
  // Enable pull-up mode
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_config(&io_conf);
}

