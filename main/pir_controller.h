#pragma once

#ifndef _WINSTON_PIR_CONTROLLER_H_
#define _WINSTON_PIR_CONTROLLER_H_

#include <string>
#include <vector>

#include "driver/gpio.h"

#include "controller.h"


// Passive-Infra-Red sensor controller, like for the AM 312.
// Uses interrupts to receive events.
// TODO: Make this more general and use for e.g. Reed relays.
class PIRController : public Controller {
 public:
  PIRController(const std::string& id, const gpio_num_t pin);
  bool init() override;
  std::vector<SensorConfig*> getSensors() override;

  // Called when a PIR interrupt event occurs.
  void onInterruptEvent();
 private:
  const std::string id_;
  const gpio_num_t pin_;
  volatile bool motion_encountered_;
  void initPin(gpio_num_t n);

  // Returns whether motion happened since the last call.
  bool motionSinceLastCall();

};

#endif /* _WINSTON_DISPLAY_CONTROLLER_H_ */
