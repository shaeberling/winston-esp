#include "pir_controller.h"

#include <string>
#include <vector>

#include "driver/gpio.h"
#include <esp_event.h>
#include <esp_log.h>

#include "controller.h"

namespace {
  // Forward C-style interrupt callbacks to class.
  static void IRAM_ATTR gpio_isr_handler(void* arg) {
    static_cast<PIRController*>(arg)->onInterruptEvent();
  };
}  // namespace


static const char *TAG = "win-pir-cntrl";

// TODO: Could we combine this and ReedController by e.g. emulating an "open"
//       condition for N seconds after the last motion event?
PIRController::PIRController(const std::string& id, const gpio_num_t pin)
    :id_(id), pin_(pin), motion_encountered_(false) {
};

// override
bool PIRController::init() {
  ESP_LOGI(TAG, "Initializing PIR sensor on GPIO %d", pin_);
  initPin(pin_);
  return true;
}

// override
void PIRController::registerIO(std::vector<SensorConfig*>* sensors,
                               std::vector<ActuatorConfig*>* actuators) {
  sensors->push_back(new SensorConfig {
    .name = "pir",
    .id = id_,
    .update_interval_seconds = 5,
    .get_value = [this](void){ return std::to_string(this->motionSinceLastCall()); }
  });
}

// private
bool PIRController::motionSinceLastCall() {
  auto result = motion_encountered_;
  motion_encountered_ = false;
  return result;
}

// private
void PIRController::onInterruptEvent() {
  // Note: Called by interrupt. Keep it short and no serial printing!
  this->motion_encountered_ = true;
}

// private 
void PIRController::initPin(gpio_num_t n) {
  ESP_LOGI(TAG, "Initializing pin %d as PIR.", n);
  gpio_config_t io_conf;
  // GPIO interrupt type: both rising and falling edge.
  io_conf.intr_type = GPIO_INTR_ANYEDGE;
  // Bit mask of the pins.
  io_conf.pin_bit_mask = 1ULL<<n;
  // Set as input mode.
  io_conf.mode = GPIO_MODE_INPUT;
  // Enable pull-up mode
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_config(&io_conf);

  // Register the interrupt 
  ESP_ERROR_CHECK(gpio_isr_handler_add(n, gpio_isr_handler, this));
}
