#include "display_controller.h"

#include "locking.h"
#include <sstream>
#include "esp_log.h"
#include "fonts.h"
#include "ssd1306.hpp"

static const char* TAG = "display-ctrl";

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

// Note: With 5 pixel-wide font, we can fit 21 chars with 2 pixel buffers.
// Note: Vertically, the color version of this display has 16 pixels of yellow,
//       a little gap and 48 blue lines.
DisplayController::DisplayController(const gpio_num_t scl,
                                     const gpio_num_t sda,
                                     Locking* locking) :
    gpio_scl_(scl),
    gpio_sda_(sda),
    locking_(locking),
    panel_type_(SSD1306_128x64),
    oled_(NULL),
    wifi_status_(DISPLAY_WIFI_NOT_CONNECTED),
    ip_address_("N/A") {
}

void DisplayController::init() {
  if (oled_ != NULL) {
    ESP_LOGE(TAG, "DisplayController already initialized.");
    return;
  }
  if (!this->locking_->lockI2C(TAG)) {
    ESP_LOGE(TAG, "Cannot lock I2C bus access.");
    return;
  }

  // These are the default GPIOs for the ESP32 WROOM board.
  // Change these if necessary.
  oled_ = new OLED(gpio_scl_, gpio_sda_, panel_type_);
  if (oled_->init()) {
    ESP_LOGI(TAG, "Display controller initialized.");
    active_ = true;
  } else {
    ESP_LOGE(TAG, "Failed to initialize OLED.");
    active_ = false;
  }
  if (!this->locking_->unlockI2C(TAG)) {
    ESP_LOGE(TAG, "Cannot unlock I2C bus access.");
    return;
  }
  update();
}

// private
void DisplayController::update() {
  if (!active_) {
    return;
  }
  if (!this->locking_->lockI2C(TAG)) {
    ESP_LOGE(TAG, "Cannot lock I2C bus access.");
    return;
  }
  oled_->clear();
  oled_->select_font(0);

  // Header.
  oled_->fill_rectangle(0, 0, oled_->get_width(), 11, WHITE);
  oled_->draw_string(2, 2, "Winston-ESP beta v0.3", BLACK, WHITE);

  // WIFI status.
  std::ostringstream wifi_str;
  wifi_str << "Wifi: " << getWifiString(this->wifi_status_);
  oled_->draw_string(2, 20, wifi_str.str(), WHITE, BLACK);

  // IP address.
  std::ostringstream ip_str;
  ip_str << "IP  : " << this->ip_address_;
  oled_->draw_string(2, 28, ip_str.str(), WHITE, BLACK);

  // Footer with time and date.
  oled_->fill_rectangle(0, 55, oled_->get_width(), 9, WHITE);
  oled_->draw_string(2, 56, date_time_, BLACK, WHITE);

  oled_->refresh(true);
  if (!this->locking_->unlockI2C(TAG)) {
    ESP_LOGE(TAG, "Cannot unlock I2C bus access.");
    return;
  }
}

void DisplayController::setWifiStatus(WifiStatus status) {
  this->wifi_status_ = status;
  update();
}

void DisplayController::setIpAddress(const std::string& address) {
  this->ip_address_ = address;
  update();
}

void DisplayController::setDateAndTime(const std::string& date_time) {
  this->date_time_ = date_time;
  update();
}