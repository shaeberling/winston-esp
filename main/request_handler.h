#pragma once

#ifndef _WINSTON_REQUEST_HANDLER_H_
#define _WINSTON_REQUEST_HANDLER_H_

#include <string>

#include "hall_effect_controller.h"
#include "reed_controller.h"
#include "relay_controller.h"
#include "request_handler.h"
#include "system_controller.h"
#include "temp_controller.h"
#include "time_controller.h"

// Routes incoming requests.
class RequestHandler {
 public:
  RequestHandler(ReedController* reed,
                 RelayController* relay,
                 TempController* temp,
                 HallEffectController* hall,
                 TimeController* time,
                 SystemController* system);
  std::string handle(const std::string& uri);
 private:
  ReedController* reed_;
  RelayController* relay_;
  TempController* temp_;
  HallEffectController* hall_;
  TimeController* time_;
  SystemController* system_;

  bool isReedClosed(const std::string& req);
  bool switchRelay(const std::string& req);
  float getTemperature(const std::string& req);
  float getHumidity(const std::string& req);
  int getHallEffect(const std::string& req);
  std::string getTime(const std::string& req);
  std::string getSystemValue(const std::string& req);
};

#endif /* _WINSTON_REQUEST_HANDLER_H_ */
