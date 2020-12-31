#pragma once

#ifndef _WINSTON_REED_CONTROLLER_H_
#define _WINSTON_REED_CONTROLLER_H_

#include <string>
#include <vector>

#include "driver/gpio.h"
#include <esp_http_server.h>

#include "controller.h"

class ReedController : public Controller {
 public:
  ReedController(const std::string& id, const gpio_num_t pin);
  bool init() override;
  void registerIO(std::vector<SensorConfig*>*,
                  std::vector<ActuatorConfig*>*) override;
  bool isClosed();

 private:
  const std::string id_;
  const gpio_num_t pin_;
};

#endif /* _WINSTON_REED_CONTROLLER_H_ */
