#pragma once

#ifndef _WINSTON_CONTROLLER_H_
#define _WINSTON_CONTROLLER_H_

#include <functional>
#include <string>
#include <vector>

// Configuration data for controllers to tell the hub on how to handle sensors.
struct SensorConfig {
  // E.g. "temp", "reed", etc.
  std::string name;
  // Unique name to this sensor, e.g. "garage1" or simply "main".
  std::string id;
  // Zero or a negative value means no update.
  int update_interval_seconds;
  // Function that will return the value of the sensor as a string.
  std::function<std::string(void)> get_value;
};

struct ActuatorConfig {
  // E.g. "temp", "reed", etc.
  std::string name;
  // Unique name to this sensor, e.g. "garage1" or simply "main".
  std::string id;
  // Function that will be called with the given data value as a string.
  std::function<bool(const std::string&)> set_value;
};

// Interface for all controllers.
class Controller {
 public:
  // Returns a list of sensor configurations.
  // Caller owns the pointers!
  virtual void registerIO(std::vector<SensorConfig*>*,
                          std::vector<ActuatorConfig*>*);

  // Initializes the controller.
  virtual bool init();

};

#endif /* _WINSTON_CONTROLLER_H_ */
