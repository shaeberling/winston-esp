#pragma once

#ifndef _WINSTON_SK6812_CONTROLLER_H_
#define _WINSTON_SK6812_CONTROLLER_H_

#include "controller.h"
#include "locking.h"
#include "driver/gpio.h"

#include "Adafruit_NeoPixel.h"

#include <string>
#include <vector>

// Controls SK6812 RGBW LED strips
// Note: SK6812 is a apparently a clone of WS2812.
//       however FastLED does not yet support RGBW, just RGB.
//       See https://github.com/FastLED/FastLED/issues/373
class SK6812Controller : public Controller {
 public:
  SK6812Controller(const std::string& id,
                   const gpio_num_t pin,
                   Locking* locking);

  bool init() override;
  void registerIO(std::vector<SensorConfig*>*,
                  std::vector<ActuatorConfig*>*) override;

  void testPattern() const;
 private:
  const std::string id_;
  const gpio_num_t pin_;
  Locking* locking_;
  Adafruit_NeoPixel* pixels_;

  bool initialized_;

  bool handleCommand(const std::string& data);
};

#endif /* _WINSTON_SK6812_CONTROLLER_H_ */