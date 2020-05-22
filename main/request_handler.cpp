#include "request_handler.h"

#include <sstream>
#include <string>
#include <vector>

#include "esp_log.h"

#include "hall_effect_controller.h"
#include "reed_controller.h"
#include "relay_controller.h"
#include "request_handler.h"
#include "system_controller.h"
#include "temp_controller.h"
#include "time_controller.h"

namespace {

static const char *TAG = "winston-req-handler";

void split(const std::string& str,
           const char delim,
           std::vector<std::string>* items) {
  std::stringstream ss(str);
  std::string token;
  while (std::getline(ss, token, delim)) {
      items->push_back(token);
  }
}

}  // namespace

RequestHandler::RequestHandler(ReedController* reed,
                               RelayController* relay,
                               TempController* temp,
                               HallEffectController* hall,
                               TimeController* time,
                               SystemController* system)
    : reed_(reed),
      relay_(relay),
      temp_(temp),
      hall_(hall),
      time_(time),
      system_(system) {
}

std::string RequestHandler::handle(const std::string& uri) {
  const std::string io_prefix = "/io";
  // Ensure request starts with this prefix.
  if (uri.rfind(io_prefix) != 0) {
    return "Bad request.";
  }
  std::string requestStr(uri.substr(io_prefix.length()));

  // TODO: Separate this out into modules.
  const std::string reed_path = "/reed/";
  const std::string relay_path = "/relay/";
  const std::string temp_path = "/temp/";
  const std::string hum_path = "/hum/";
  const std::string hall_path = "/hall/";
  const std::string system_path = "/system/";

  if (requestStr.rfind(reed_path, 0) == 0) {
    std::string req_data(requestStr.substr(reed_path.length()));
    return isReedClosed(req_data) ? "1" : "0";
  } else if (requestStr.rfind(relay_path, 0) == 0) {
    std::string req_data(requestStr.substr(relay_path.length()));
    bool success = switchRelay(req_data);
    return success ? "OK" : "FAIL";
  } else if (requestStr.rfind(temp_path, 0) == 0) {
    std::string req_data(requestStr.substr(temp_path.length()));
    float temperature = getTemperature(req_data);
    return std::to_string(temperature);
  } else if (requestStr.rfind(hum_path, 0) == 0) {
    std::string req_data(requestStr.substr(hum_path.length()));
    float humidity = getHumidity(req_data);
    return std::to_string(humidity);
  } else if (requestStr.rfind(hall_path, 0) == 0) {
    std::string req_data(requestStr.substr(hall_path.length()));
    int value = getHallEffect(req_data);
    return std::to_string(value);
  } else if (requestStr.rfind(system_path, 0) == 0) {
    std::string req_data(requestStr.substr(system_path.length()));
    return getSystemValue(req_data);
  } else {
    return "Hello, Winston ESP here.";
  }
}

// /io/reed/[idx]
bool RequestHandler::isReedClosed(const std::string& req) {
  ESP_LOGI(TAG, "Geet reed status for '%s'", req.c_str());
  char* pEnd = NULL;
  int reed_idx = strtod(req.c_str(), &pEnd);
  if (*pEnd) {
    ESP_LOGW(TAG, "Invalid reed index.");
    return false;
  }
  return reed_->is_closed(reed_idx);
}

// /io/relay/[idx]/[0,1,2,3]
// @param req: E.g. "0/1", 1/0", "3/0/10000"
// 0 = TURN OFF
// 1 = TURN ON
// 2 = CLICK WITH DEFAULT DELAY
// 3 = CLICK WITH SPECIFIED DELAY (IN MILLIS).
bool RequestHandler::switchRelay(const std::string& req) {
  ESP_LOGI(TAG, "Switch relay '%s'", req.c_str());

  // Extract the arguments.
  std::vector<std::string> args;
  split(req, '/', &args);

  // Format at this point should be  <relay>/<on>[/<params>]
  if (args.size() < 2) {
    ESP_LOGW(TAG, "Invalid relay switch request: Missing switch component.");
    return false;
  }

  const std::string& relay_idx_str = args[0];
  ESP_LOGI(TAG, "Switch relay idx '%s'", relay_idx_str.c_str());
  char* pEnd = NULL;
  int relay_idx = strtod(relay_idx_str.c_str(), &pEnd);
  if (*pEnd) {
    ESP_LOGW(TAG, "Invalid relay index.");
    return false;
  }

  const std::string& relay_switch_cmd = args[1];
  ESP_LOGI(TAG, "Switch command is '%s'", relay_switch_cmd.c_str());
  if (relay_switch_cmd == "0") {
    return relay_->switch_on(relay_idx, false);
  } else if (relay_switch_cmd == "1") {
    return relay_->switch_on(relay_idx, true);
  } else if (relay_switch_cmd == "2") {
    return relay_->click(relay_idx);
  } else if (relay_switch_cmd == "3") {
    if (args.size() < 3) {
      ESP_LOGW(TAG, "Invalid relay switch request: Command '3' needs param.");
      return false;
    }

    // Parse the "click" parameter (the third request argument).
    pEnd = NULL;
    int click_delay_millis = strtod(args[2].c_str(), &pEnd);
    if (*pEnd) {
      ESP_LOGW(TAG, "Invalid 'click' parmeter. Must be an int.");
      return false;
    }
    return relay_->click(relay_idx, click_delay_millis);
  } else {
    ESP_LOGW(TAG, "Invalid relay switch value.");
    return false;
  }
}

// /io/temp/[idx]
float RequestHandler::getTemperature(const std::string& req) {
  ESP_LOGI(TAG, "Get temperature '%s'", req.c_str());

  // TODO: Turn this into a method, return an optional/null.
  char* pEnd = NULL;
  int temp_idx = strtod(req.c_str(), &pEnd);
  if (*pEnd) {
    ESP_LOGW(TAG, "Cannot parse temperature sensor index.");
    return -1;
  }
  return temp_->getCelsius(temp_idx);
}

// /io/hum/[idx]
float RequestHandler::getHumidity(const std::string& req) {
  ESP_LOGI(TAG, "Get humidity '%s'", req.c_str());

  // TODO: Turn this into a method, return an optional/null.
  char* pEnd = NULL;
  int temp_idx = strtod(req.c_str(), &pEnd);
  if (*pEnd) {
    ESP_LOGW(TAG, "Cannot parse humidty sensor index.");
    return -1;
  }
  return temp_->getHumidity(temp_idx);
}

// /io/hall/[idx]
int RequestHandler::getHallEffect(const std::string& req) {
  ESP_LOGI(TAG, "Get hall effect '%s'", req.c_str());

  // TODO: Turn this into a method, return an optional/null.
  char* pEnd = NULL;
  int hall_idx = strtod(req.c_str(), &pEnd);
  if (*pEnd) {
    ESP_LOGW(TAG, "Cannot parse hall efect sensor index.");
    return -1;
  }
  return hall_->getValue(hall_idx);
}

// /io/sys/[parameter]
std::string RequestHandler::getSystemValue(const std::string& req) {
  if (req.find("heap") == 0) {
    return std::to_string(system_->getFreeHeapBytes() / 1024) +  " KB";
  } else if (req.find("time") == 0) {
    return time_->getDateAndTime();
  } else if (req.find("stats") == 0) {
    return system_->getRunTimeStats();
  } else {
    return "Unknown parameter for 'system'.";
  }
}
