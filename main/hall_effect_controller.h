#pragma once

#ifndef _WINSTON_HALL_EFFECT_CONTROLLER_H_
#define _WINSTON_HALL_EFFECT_CONTROLLER_H_

#include <esp_http_server.h>
#include <vector>

// Controls relays that are active-low. */
class HallEffectController {
 public:
  HallEffectController();
  int getValue(int idx);
  void init();
 private:
  bool initialized_;
};

#endif /* _WINSTON_HALL_EFFECT_CONTROLLER_H_ */
