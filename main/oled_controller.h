#pragma once

#ifndef _WINSTON_OLED_CONTROLLER_H_
#define _WINSTON_OLED_CONTROLLER_H_

#include "ssd1306.hpp"

enum WifiStatus {DISPLAY_WIFI_CONNECTED,
                 DISPLAY_WIFI_CONNECTING,
                 DISPLAY_WIFI_NOT_CONNECTED
};

// Controls relays that are active-low. */
class OledController {
 public:
  OledController();
  void init();
  void setWifiStatus(WifiStatus status);
  void setIpAddress(const std::string& address);
 private:
  OLED* oled_;
  bool active_;

  WifiStatus wifi_status_;
  std::string ip_address_;

  void update();
};

#endif /* _WINSTON_OLED_CONTROLLER_H_ */
