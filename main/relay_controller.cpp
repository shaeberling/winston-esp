#include "relay_controller.h"

#include <sstream>
#include <string>
#include <vector>

#include "esp_log.h"
#include "driver/gpio.h"

#define DEFAULT_CLICK_DELAY_MILLIS 500

static const char *TAG = "win-relay-ctrl";

namespace {

void split(const std::string& str,
           const char delim,
           std::vector<std::string>* items) {
  std::stringstream ss(str);
  std::string token;
  while (std::getline(ss, token, delim)) {
    items->push_back(token);
  }
}

}  // namespace

RelayController::RelayController(const std::string& id, const gpio_num_t pin)
    : id_(id), pin_(pin) {
}

// override
bool RelayController::init() {
  gpio_config_t io_conf;
  // No interrupt.
  io_conf.intr_type = GPIO_INTR_DISABLE;
  // Bit mask of the pins.
  io_conf.pin_bit_mask = 1ULL<<pin_;
  // Set as output mode.
  io_conf.mode = GPIO_MODE_OUTPUT;
  // Enable pull-up mode
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  // Ensure that pin is pulled high before initialization. If this is
  // skipped, it will cause the pin to be pulled low (to ground) which
  // causes an acive-low relay to turn on, which would be bad!
  gpio_set_level((gpio_num_t) pin_, 1);
  gpio_config(&io_conf);
  return true;
}

// override
void RelayController::registerIO(std::vector<SensorConfig*>* sensors,
                                 std::vector<ActuatorConfig*>* actuators) {
  actuators->push_back(new ActuatorConfig {
    .name = "relay",
    .id = this->id_,
    .set_value = [this](const std::string& data) {
        return this->handleCommand(data);
    }
  });
}

// @param req: E.g. "0/1", 1/0", "3/0/10000"
// 0 = TURN OFF
// 1 = TURN ON
// 2 = CLICK WITH DEFAULT DELAY
// 3 = CLICK WITH SPECIFIED DELAY (IN MILLIS).
// private
bool RelayController::handleCommand(const std::string& data) {
  // Extract the arguments.
  std::vector<std::string> args;
  split(data, '/', &args);

  // Format at this point should be  <relay>/<on>[/<params>]
  if (args.size() == 0) {
    ESP_LOGW(TAG, "Invalid relay switch request: Missing switch component.");
    return false;
  }

  const std::string& relay_switch_cmd = args[0];
  ESP_LOGI(TAG, "Switch command is '%s'", relay_switch_cmd.c_str());
  if (relay_switch_cmd == "0") {
    return switch_on(false);
  } else if (relay_switch_cmd == "1") {
    return switch_on(true);
  } else if (relay_switch_cmd == "2") {
    return click();
  } else if (relay_switch_cmd == "3") {
    if (args.size() < 2) {
      ESP_LOGW(TAG, "Invalid relay switch request: Command '3' needs param.");
      return false;
    }

    // Parse the "click" parameter (the third request argument).
    char* pEnd = NULL;
    int click_delay_millis = strtod(args[1].c_str(), &pEnd);
    if (*pEnd) {
      ESP_LOGW(TAG, "Invalid 'click' parmeter. Must be an int.");
      return false;
    }
    return click(click_delay_millis);
  } else {
    ESP_LOGW(TAG, "Invalid relay switch value.");
    return false;
  }
}

// public 
bool RelayController::switch_on(bool on) {
  ESP_LOGI(TAG, "Swtching relay %s %s", id_.c_str(), (on ? "ON" : "OFF"));

  // Note, all of this logic here is for relays that are switched by
  // pulling them down (active-low).
  gpio_set_level(pin_, (on ? 0 : 1));
  return true;
}

// public 
bool RelayController::click() {
  return click(DEFAULT_CLICK_DELAY_MILLIS);
}

// public 
bool RelayController::click(int delay_millis) {
  if (delay_millis <= 0) {
    ESP_LOGW(TAG, "Click delay must be positive. Was: '%d'", delay_millis);
    return false;
  }

  ESP_LOGI(TAG, "Clicking relay %s for %d millis.", id_.c_str(), delay_millis);
  // Turn switch on, wait, turn it back off.
  switch_on(true);
  vTaskDelay(delay_millis / portTICK_PERIOD_MS);
  switch_on(false);
  return true;
}
