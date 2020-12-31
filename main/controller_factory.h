#pragma once

#ifndef _WINSTON_TIME_CONTROLLER_FACTORY_H_
#define _WINSTON_TIME_CONTROLLER_FACTORY_H_

#include <string>

#include "controller.h"
#include "device_settings.pb.h"

class Locking;
class TimeController;
class SystemController;

class ControllerFactory {
 public:
  explicit ControllerFactory(Locking* locking,
                             TimeController* time_controller,
                             SystemController* system_controller);

  // Caller owns the returned pointer.
  Controller* createController(const ComponentProto& component);

 private:
  Locking* locking_;
  TimeController* time_controller_;
  SystemController* system_controller_;
};

#endif /* _WINSTON_TIME_CONTROLLER_FACTORY_H_ */
