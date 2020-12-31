#pragma once

#ifndef _WINSTON_CONTROL_HUB_H_
#define _WINSTON_CONTROL_HUB_H_

#include "controller.h"

#include <functional>
#include <string>
#include <vector>


class ControlHub;
class TimeController;

// The control hub is responsible for forwarding event between sensor, actuators
// and their consumers, like MQTT, display and the webserver.

class ControlHub {
 public:
  ControlHub();
  // Registers a controller, given its sensors and actuators.
  void registerController(Controller* controller);
 private:
  const TimeController* time_controller_;

  void registerSensor(SensorConfig* config);
  void registerActuator(ActuatorConfig* config);
  void onSensorUpdate(const std::string& path, const std::string& value);
  static void startUpdateLoop(void* param);
};



#endif /* _WINSTON_CONTROL_HUB_H_ */
