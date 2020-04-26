#pragma once

#ifndef _WINSTON_TEMP_CONTROLLER_H_
#define _WINSTON_TEMP_CONTROLLER_H_

#include "htu21d_controller.h"

// Get temperature from the built-in ESP32 temp sensor.
class TempController {
 public:
  TempController(HTU21DController* htu21d_controller);
  float getCelsius(int idx);
  float getHumidity(int idx);

 private:
  HTU21DController* htu21d_controller_;
};

#endif /* _WINSTON_TEMP_CONTROLLER_H_ */
