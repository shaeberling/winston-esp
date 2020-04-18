#include "oled_controller.h"

#include <sstream>
#include "esp_log.h"
#include "fonts.h"
#include "ssd1306.hpp"

static const char *TAG = "hall-controller";

namespace {
static const char* wifiStrings[] = {
  "Connected :-)",
  "Connecting...",
  "Not connected"
};
const char* getWifiString(int val) {
  return wifiStrings[val];
};
}

// Note: the display has 16 pixels of yellow, a little gap and 48 blue lines. 
//       With 5 pixel-wide font, we can fit 21 chars with 2 pixel buffers.
OledController::OledController() :
    wifi_status_(DISPLAY_WIFI_NOT_CONNECTED),
    ip_address_("N/A") {
}

void OledController::init() {
  // These are the default GPIOs for the ESP32 WROOM board.
  // Change these if necessary.
  oled_ = new OLED(GPIO_NUM_22, GPIO_NUM_21, SSD1306_128x64);
  if (oled_->init()) {
    ESP_LOGI(TAG, "OLED controller initialized.");
    active_ = true;
  } else {
    ESP_LOGE(TAG, "Failed to initialize OLED.");
    active_ = false;
  }
  update();
}

// private
void OledController::update() {
  if (!active_) {
    return;
  }
  oled_->clear();
  oled_->select_font(0);
  oled_->fill_rectangle(0, 0, oled_->get_width(), 16, WHITE);
  oled_->draw_string(2, 4, "Winston-ESP      v0.1", BLACK, WHITE);
  std::ostringstream wifi_str;
  wifi_str << "Wifi: " << getWifiString(this->wifi_status_);
  oled_->draw_string(2, 20, wifi_str.str(), WHITE, BLACK);
  std::ostringstream ip_str;
  ip_str << "IP  : " << this->ip_address_;
  oled_->draw_string(2, 28, ip_str.str(), WHITE, BLACK);
  oled_->refresh(true);
}

void OledController::setWifiStatus(WifiStatus status) {
  this->wifi_status_ = status;
  update();
}

void OledController::setIpAddress(const std::string& address) {
  this->ip_address_ = address;
  update();
}
