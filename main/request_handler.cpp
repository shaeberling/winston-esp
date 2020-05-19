#include "request_handler.h"

#include <sstream>
#include <string>
#include <vector>

#include "esp_log.h"

#include "hall_effect_controller.h"
#include "reed_controller.h"
#include "relay_controller.h"
#include "request_handler.h"
#include "temp_controller.h"


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

RequestHandler::RequestHandler(ReedController* reed_controller,
                               RelayController* relay_controller,
                               TempController* temp_controller,
                               HallEffectController* hall_controller)
    : reed_controller_(reed_controller),
      relay_controller_(relay_controller),
      temp_controller_(temp_controller),
      hall_controller_(hall_controller) {
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

  if (requestStr.rfind(reed_path, 0) == 0) {
    std::string req_data(requestStr.substr(reed_path.length()));
    return is_reed_closed(req_data) ? "1" : "0";
  } else if (requestStr.rfind(relay_path, 0) == 0) {
    std::string req_data(requestStr.substr(relay_path.length()));
    bool success = switch_relay(req_data);
    return success ? "OK" : "FAIL";
  } else if (requestStr.rfind(temp_path, 0) == 0) {
    std::string req_data(requestStr.substr(temp_path.length()));
    float temperature = get_temperature(req_data);
    return std::to_string(temperature);
  } else if (requestStr.rfind(hum_path, 0) == 0) {
    std::string req_data(requestStr.substr(hum_path.length()));
    float humidity = get_humidity(req_data);
    return std::to_string(humidity);
  } else if (requestStr.rfind(hall_path, 0) == 0) {
    std::string req_data(requestStr.substr(hall_path.length()));
    int value = get_hall_effect(req_data);
    return std::to_string(value);
  } else {
    return "Hello, Winston ESP here.";
  }
}

// /io/reed/[idx]
bool RequestHandler::is_reed_closed(const std::string& req) {
  ESP_LOGI(TAG, "Geet reed status for '%s'", req.c_str());
  char* pEnd = NULL;
  int reed_idx = strtod(req.c_str(), &pEnd);
  if (*pEnd) {
    ESP_LOGW(TAG, "Invalid reed index.");
    return false;
  }
  return reed_controller_->is_closed(reed_idx);
}

// /io/relay/[idx]/[0,1,2,3]
// @param req: E.g. "0/1", 1/0", "3/0/10000"
// 0 = TURN OFF
// 1 = TURN ON
// 2 = CLICK WITH DEFAULT DELAY
// 3 = CLICK WITH SPECIFIED DELAY (IN MILLIS).
bool RequestHandler::switch_relay(const std::string& req) {
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
    return relay_controller_->switch_on(relay_idx, false);
  } else if (relay_switch_cmd == "1") {
    return relay_controller_->switch_on(relay_idx, true);
  } else if (relay_switch_cmd == "2") {
    return relay_controller_->click(relay_idx);
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
    return relay_controller_->click(relay_idx, click_delay_millis);
  } else {
    ESP_LOGW(TAG, "Invalid relay switch value.");
    return false;
  }
}

// /io/temp/[idx]
float RequestHandler::get_temperature(const std::string& req) {
  ESP_LOGI(TAG, "Get temperature '%s'", req.c_str());

  // TODO: Turn this into a method, return an optional/null.
  char* pEnd = NULL;
  int temp_idx = strtod(req.c_str(), &pEnd);
  if (*pEnd) {
    ESP_LOGW(TAG, "Cannot parse temperature sensor index.");
    return -1;
  }
  return temp_controller_->getCelsius(temp_idx);
}

// /io/hum/[idx]
float RequestHandler::get_humidity(const std::string& req) {
  ESP_LOGI(TAG, "Get humidity '%s'", req.c_str());

  // TODO: Turn this into a method, return an optional/null.
  char* pEnd = NULL;
  int temp_idx = strtod(req.c_str(), &pEnd);
  if (*pEnd) {
    ESP_LOGW(TAG, "Cannot parse humidty sensor index.");
    return -1;
  }
  return temp_controller_->getHumidity(temp_idx);
}

// /io/hall/[idx]
int RequestHandler::get_hall_effect(const std::string& req) {
  ESP_LOGI(TAG, "Get hall effect '%s'", req.c_str());

  // TODO: Turn this into a method, return an optional/null.
  char* pEnd = NULL;
  int hall_idx = strtod(req.c_str(), &pEnd);
  if (*pEnd) {
    ESP_LOGW(TAG, "Cannot parse hall efect sensor index.");
    return -1;
  }
  return hall_controller_->getValue(hall_idx);
}
