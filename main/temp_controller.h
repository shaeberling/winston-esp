#pragma once

#ifndef _WINSTON_TEMP_CONTROLLER_H_
#define _WINSTON_TEMP_CONTROLLER_H_

#include "control_hub.h"
#include "controller.h"
#include "htu21d_controller.h"

#include <functional>
#include <string>
#include <vector>

// Get temperature from the built-in ESP32 temp sensor.
class TempController : public Controller {
 public:
  TempController(HTU21DController* htu21d_controller);
  std::vector<SensorConfig*> getSensors() const override;

  float getCelsius(int idx) const;
  float getHumidity(int idx) const;

 private:
  HTU21DController* htu21d_controller_;
};

#endif /* _WINSTON_TEMP_CONTROLLER_H_ */
