#include "relay_controller.h"

#include "esp_log.h"
#include "driver/gpio.h"

#define CLICK_DELAY_MILLIS 500

static const char *TAG = "relay-controller";

RelayController::RelayController(const std::vector<int>& mapping) : mapping_(mapping) {
  for (int pin : mapping_) {
    initPin(pin);
  }
}

// public 
bool RelayController::switch_on(int idx, bool on) {
  if (mapping_.size() <= idx) {
    ESP_LOGW(TAG, "Illegal relay index '%d'", idx);
    return false;
  }
  ESP_LOGI(TAG, "Swtching relay %d %s", idx, (on ? "ON" : "OFF"));

  // Note, all of this logic here is for relays that are switched by
  // pulling them down (active-low).
  gpio_set_level((gpio_num_t) mapping_[idx], (on ? 0 : 1));
  return true;
}

// public 
bool RelayController::click(int idx) {
  if (mapping_.size() <= idx) {
    ESP_LOGW(TAG, "Illegal relay index '%d'", idx);
    return false;
  }
  // Turn switch on, wait, turn it back off.
  switch_on(idx, true);
  vTaskDelay(CLICK_DELAY_MILLIS / portTICK_PERIOD_MS);
  switch_on(idx, false);
  return true;
}

// private
void RelayController::initPin(int n) {
  ESP_LOGI(TAG, "Initializing pin %d as RELAY.", n);
  gpio_config_t io_conf;
  // No interrupt.
  io_conf.intr_type = GPIO_INTR_DISABLE;
  // Bit mask of the pins.
  io_conf.pin_bit_mask = 1ULL<<n;
  // Set as output mode.
  io_conf.mode = GPIO_MODE_OUTPUT;
  // Enable pull-up mode
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  // Ensure that pin is pulled high before initialization. If this is
  // skipped, it will cause the pin to be pulled low (to ground) which
  // causes an acive-low relay to turn on, which would be bad!
  gpio_set_level((gpio_num_t) n, 1);
  gpio_config(&io_conf);
}
