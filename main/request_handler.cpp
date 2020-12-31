#include "request_handler.h"

#include <sstream>
#include <string>
#include <vector>

#include "esp_log.h"

#include "system_controller.h"
#include "time_controller.h"

RequestHandler::RequestHandler(TimeController* time,
                               SystemController* system)
    : time_(time),
      system_(system) {
}

std::string RequestHandler::handle(const std::string& uri) {
  const std::string io_prefix = "/io";
  // Ensure request starts with this prefix.
  if (uri.rfind(io_prefix) != 0) {
    return "Bad request.";
  }
  std::string requestStr(uri.substr(io_prefix.length()));
  const std::string system_path = "/system/";

  if (requestStr.rfind(system_path, 0) == 0) {
    std::string req_data(requestStr.substr(system_path.length()));
    return getSystemValue(req_data);
  }
  return "Hello, Winston ESP here.";
}

// /io/system/[parameter]
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