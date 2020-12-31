#pragma once

#ifndef _WINSTON_RELAY_CONTROLLER_H_
#define _WINSTON_RELAY_CONTROLLER_H_

#include <vector>

#include <esp_http_server.h>
#include "driver/gpio.h"

#include "controller.h"

// Controls relays that are active-low. */
class RelayController : public Controller {
 public:
  RelayController(const std::string& id, const gpio_num_t pin);
  bool init() override;
  void registerIO(std::vector<SensorConfig*>*,
                  std::vector<ActuatorConfig*>*) override;

  bool switch_on(bool on);
  // Clicks with a default delay.
  bool click();
  // Clicks with given delay in milliseconds.
  bool click(int delay_millis);
 private:
  const std::string id_;
  const gpio_num_t pin_;

  bool handleCommand(const std::string& data);
};

#endif /* _WINSTON_RELAY_CONTROLLER_H_ */
