#pragma once

#ifndef _WINSTON_DISPLAY_CONTROLLER_H_
#define _WINSTON_DISPLAY_CONTROLLER_H_

#include "locking.h"
#include "driver/gpio.h"
#include "ssd1306.hpp"

enum WifiStatus {DISPLAY_WIFI_CONNECTED,
                 DISPLAY_WIFI_CONNECTING,
                 DISPLAY_WIFI_NOT_CONNECTED
};

// Controls relays that are active-low. */
class DisplayController {
 public:
  DisplayController(const gpio_num_t scl,
                    const gpio_num_t sda,
                    Locking* locking_);
  void init();
  void setWifiStatus(WifiStatus status);
  void setIpAddress(const std::string& address);
  void setMacAddress(const std::string& mac);
  void setDateAndTime(const std::string& date_time);
  void setFreeHeapBytes(int free_bytes);
  void setTemperature(float celsius);
  void setHumidity(float rel_hum);

  // Updates the display, rendering the current data.
  void update();

 private:
  const gpio_num_t gpio_scl_;
  const gpio_num_t gpio_sda_;
  Locking* locking_;

  const ssd1306_panel_type_t panel_type_;
  OLED* oled_;
  bool active_;

  WifiStatus wifi_status_;
  std::string ip_address_;
  std::string mac_address_;
  std::string date_time_;
  std::string heap_free_msg_;
  float temp_celsius_;
  float rel_humidity_;
};

#endif /* _WINSTON_DISPLAY_CONTROLLER_H_ */
