#pragma once

#ifndef _WINSTON_REQUEST_HANDLER_H_
#define _WINSTON_REQUEST_HANDLER_H_

#include <string>

#include "hall_effect_controller.h"
#include "htu21d_controller.h"
#include "reed_controller.h"
#include "relay_controller.h"
#include "request_handler.h"
#include "system_controller.h"
#include "time_controller.h"

// Routes incoming requests.
class RequestHandler {
 public:
  RequestHandler(TimeController* time,
                 SystemController* system);
  std::string handle(const std::string& uri);
 private:
  TimeController* time_;
  SystemController* system_;

  std::string getSystemValue(const std::string& req);
};

#endif /* _WINSTON_REQUEST_HANDLER_H_ */
