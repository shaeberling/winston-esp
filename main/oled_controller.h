#pragma once

#ifndef _WINSTON_OLED_CONTROLLER_H_
#define _WINSTON_OLED_CONTROLLER_H_

#include "ssd1306.hpp"

// Controls relays that are active-low. */
class OledController {
 public:
  OledController();
 private:
  OLED* oled_;
  bool active_;
};

#endif /* _WINSTON_OLED_CONTROLLER_H_ */
