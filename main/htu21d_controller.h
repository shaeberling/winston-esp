#pragma once

#ifndef _WINSTON_HTU21D_CONTROLLER_H_
#define _WINSTON_HTU21D_CONTROLLER_H_

#include "locking.h"
#include "driver/gpio.h"

// Controls relays that are active-low. */
class HTU21DController {
 public:
  HTU21DController(const gpio_num_t scl,
                   const gpio_num_t sda,
                   Locking* locking);
  bool init();
  float getCelsius();
  float getHumidity();
 private:
  const gpio_num_t gpio_scl_;
  const gpio_num_t gpio_sda_;
  Locking* locking_;

  bool initialized_;

  int getRawWithLock(int command);
  int getRaw(int command);
};

#endif /* _WINSTON_HTU21D_CONTROLLER_H_ */
