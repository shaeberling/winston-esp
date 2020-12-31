#include "controller_factory.h"

#include <string>

#include "esp_log.h"
#include "driver/gpio.h"

#include "controller.h"
#include "device_settings.pb.h"
#include "esp_temp_controller.h"
#include "htu21d_controller.h"
#include "locking.h"
#include "pir_controller.h"
#include "reed_controller.h"
#include "system_controller.h"
#include "time_controller.h"
#include "ui_controller.h"

static const char* TAG = "win-ctrl-fact";

namespace {


}  // namespace


ControllerFactory::ControllerFactory(Locking* locking,
                                     TimeController* time_controller,
                                     SystemController* system_controller) :
    locking_(locking),
    time_controller_(time_controller),
    system_controller_(system_controller) { }

Controller* ControllerFactory::createController(const ComponentProto& comp) {
  auto gpio_0 = static_cast<gpio_num_t>(comp.gpio_pin[0]);
  auto gpio_1 = static_cast<gpio_num_t>(comp.gpio_pin[1]);

  if (strcmp(comp.name, "htu21d") == 0) {
    return new HTU21DController(comp.id, gpio_0, gpio_1, locking_);
  } else if (strcmp(comp.name, "esp-temp") == 0) {
    return new EspTempController(comp.id);
  } else if (strcmp(comp.name, "ssd1306") == 0) {
    return new UiController(new DisplayController(gpio_0, gpio_1, locking_),
                            time_controller_, system_controller_);
  } else if (strcmp(comp.name, "pir") == 0) {
    return new PIRController(comp.id, gpio_0);
  } else if (strcmp(comp.name, "reed") == 0) {
    return new ReedController(comp.id, gpio_0);
  } else if (strcmp(comp.name, "relay") == 0) {
  } else {
    ESP_LOGE(TAG, "Unknown component name '%s' for '%s'.",
             comp.name, comp.id);
  }
  return NULL;
}

// TODO:

  // reed_controller = new ReedController({ 14, 27 });
  // pir_controller = new PIRController(GPIO_NUM_4);
  // relay_controller = new RelayController({ 26, 25 });
  // hall_controller = new HallEffectController();
