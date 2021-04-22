#pragma once

#ifndef _WINSTON_HTU21D_CONTROLLER_H_
#define _WINSTON_HTU21D_CONTROLLER_H_

#include "controller.h"
#include "locking.h"
#include "driver/gpio.h"

#include <string>
#include <vector>

// Controls relays that are active-low. */
class HTU21DController : public Controller {
 public:
  HTU21DController(const std::string& id,
                   const gpio_num_t scl,
                   const gpio_num_t sda,
                   Locking* locking);

  bool init() override;
  void registerIO(std::vector<SensorConfig*>*,
                  std::vector<ActuatorConfig*>*) override;

  float getCelsius() const;
  float getHumidity() const;
 private:
  const std::string id_;
  const gpio_num_t gpio_scl_;
  const gpio_num_t gpio_sda_;
  Locking* locking_;

  bool initialized_;

  bool initInternal();
  int getRawWithLock(int command) const;
  int getRaw(int command) const;
};

#endif /* _WINSTON_HTU21D_CONTROLLER_H_ */
