#pragma once

#ifndef _WINSTON_PIR_CONTROLLER_H_
#define _WINSTON_PIR_CONTROLLER_H_

#include "driver/gpio.h"

// Passive-Infra-Red sensor controller, like for the AM 312.
// Uses interrupts to receive events.
// TODO: Make this more general and use for e.g. Reed relays.
class PIRController {
 public:
  PIRController(const gpio_num_t pin);
  void init();
  // Returns number of motion events since startup.
  int getMotionCount();
  // Called when a PIR interrupt event occurs.
  void onInterruptEvent();
 private:
  const gpio_num_t pin_;
  volatile int counter_;
  void initPin(gpio_num_t n);
};

#endif /* _WINSTON_DISPLAY_CONTROLLER_H_ */
