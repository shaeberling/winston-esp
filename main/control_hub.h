#pragma once

#ifndef _WINSTON_CONTROL_HUB_H_
#define _WINSTON_CONTROL_HUB_H_

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "esp_event.h"

#include "controller.h"

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
  std::map<std::string, std::function<bool(const std::string&)>> actuators_;

  void registerSensor(SensorConfig* config);
  void registerActuator(ActuatorConfig* config);
  void onSensorUpdate(const std::string& path, const std::string& value);
  void handleActuatorEvent(esp_event_base_t event_base, 
                           int32_t event_id, void* event_data);
  static void actuator_event_handler(void* arg, esp_event_base_t event_base, 
                                     int32_t event_id, void* event_data);
  static void startUpdateLoop(void* param);
};



#endif /* _WINSTON_CONTROL_HUB_H_ */
