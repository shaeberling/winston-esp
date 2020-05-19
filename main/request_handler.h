#pragma once

#ifndef _WINSTON_REQUEST_HANDLER_H_
#define _WINSTON_REQUEST_HANDLER_H_

#include <string>

#include "hall_effect_controller.h"
#include "reed_controller.h"
#include "relay_controller.h"
#include "request_handler.h"
#include "temp_controller.h"

// Routes incoming requests.
class RequestHandler {
 public:
  RequestHandler(ReedController* reed_controller,
                 RelayController* relay_controller,
                 TempController* temp_controller,
                 HallEffectController* hall_controller);
  std::string handle(const std::string& uri);
 private:
  ReedController* reed_controller_;
  RelayController* relay_controller_;
  TempController* temp_controller_;
  HallEffectController* hall_controller_;

  bool is_reed_closed(const std::string& req);
  bool switch_relay(const std::string& req);
  float get_temperature(const std::string& req);
  float get_humidity(const std::string& req);
  int get_hall_effect(const std::string& req);
};

#endif /* _WINSTON_REQUEST_HANDLER_H_ */
