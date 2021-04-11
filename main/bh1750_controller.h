#pragma once

#ifndef _WINSTON_BH1750_CONTROLLER_H_
#define _WINSTON_BH1750_CONTROLLER_H_

#include "controller.h"
#include "locking.h"
#include "driver/gpio.h"

#include <string>
#include <vector>

// Controls relays that are active-low. */
class BH1750Controller : public Controller {
 public:
  BH1750Controller(const std::string& id,
                   const gpio_num_t scl,
                   const gpio_num_t sda,
                   Locking* locking);

  bool init() override;
  void registerIO(std::vector<SensorConfig*>*,
                  std::vector<ActuatorConfig*>*) override;

  float getLux() const;
 private:
  const std::string id_;
  const gpio_num_t gpio_scl_;
  const gpio_num_t gpio_sda_;
  Locking* locking_;

  bool initialized_;
};

#endif /* _WINSTON_BH1750_CONTROLLER_H_ */
