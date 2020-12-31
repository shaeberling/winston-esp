#include "request_handler.h"

#include <sstream>
#include <string>
#include <vector>

#include "esp_log.h"

#include "system_controller.h"
#include "time_controller.h"

static const char *TAG = "winston-req-handler";

RequestHandler::RequestHandler(TimeController* time,
                               SystemController* system)
    : time_(time),
      system_(system) {
}

std::string RequestHandler::handle(const std::string& uri) {
  ESP_LOGI(TAG, "HTTP request incoming for %s", uri.c_str());
  return "Hello, Winston ESP here.";
}

// /io/sys/[parameter]
std::string RequestHandler::getSystemValue(const std::string& req) {
  if (req.find("heap") == 0) {
    return std::to_string(system_->getFreeHeapBytes() / 1024) +  " KB";
  } else if (req.find("time") == 0) {
    return time_->getDateAndTime();
  } else if (req.find("stats") == 0) {
    return system_->getRunTimeStats();
  } else if (req.find("info") == 0) {
    return system_->getSystemInfo();
  } else if (req.find("restart") == 0) {
    // Restart in 5 seconds so that we can first serve the response.
    // If the browser waits for the response, it might request it again
    // and again, resulting in a boot loop. Granted, this should not be
    // an issue if this is accessed through a REST API.
    return system_->restart(5000) ? "OK" : "ERROR";
  } else {
    return "Unknown parameter for 'system'.";
  }
}