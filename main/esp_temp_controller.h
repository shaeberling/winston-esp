#pragma once

#ifndef _WINSTON_TEMP_CONTROLLER_H_
#define _WINSTON_TEMP_CONTROLLER_H_

#include "controller.h"

#include <string>
#include <vector>

// Get temperature from the built-in ESP32 temp sensor.
class EspTempController : public Controller {
 public:
  EspTempController(const std::string& id);
  bool init() override;
  std::vector<SensorConfig*> getSensors() const override;

 private:
  std::string id_;
};

#endif /* _WINSTON_TEMP_CONTROLLER_H_ */
