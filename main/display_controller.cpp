#include "display_controller.h"

#include "locking.h"
#include <sstream>
#include "esp_log.h"
#include "fonts.h"
#include "ssd1306.hpp"

static const char* TAG = "win-disp-ctrl";

namespace {
static const char* wifiStrings[] = {
  "Connected :-)",
  "Connecting...",
  "Not connected"
};
const char* getWifiString(int val) {
  return wifiStrings[val];
};

void customSpacingText(OLED* oled,
                       int start_x,
                       int y,
                       const std::string& text,
                       char ch,
                       int sp_reduction_left,
                       int sp_reduction_right) {
  // First paint the colons. Since we're making them slimmer,
  // if we painted them later they would erase pixels within
  // their dimensions that belong to other characters.
  int x = start_x;
  for (int i = 0; i < text.length(); ++i) {
    if (text[i] == ch) x -= sp_reduction_left;
    if (text[i] == ch) {
      oled->draw_char(x, y, text[i], WHITE, BLACK);
    }
    x += 6;
    if (text[i] == ch) x -= sp_reduction_right;
  }
  x = start_x;
  for (int i = 0; i < text.length(); ++i) {
    if (text[i] == ch) x -= sp_reduction_left;
    if (text[i] != ch) {
      oled->draw_char(x, y, text[i], WHITE, BLACK);
    }
    x += 6;
    if (text[i] == ch) x -= sp_reduction_right;
  }
};

void customSpacingText(OLED* oled,
                       int start_x,
                       int y,
                       const std::string& text,
                       char ch,
                       int sp_reduction) {
  customSpacingText(oled, start_x, y, text, ch, sp_reduction, sp_reduction);
};

}  // namespace

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
    node_name_("[node name]"),
    wifi_status_(DISPLAY_WIFI_NOT_CONNECTED),
    ip_address_("N/A"),
    mac_address_("00:00:00:00:00:00"),
    date_time_(""),
    heap_free_msg_(""),
    temp_celsius_(0),
    rel_humidity_(0) {
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
  oled_->draw_string(2, 2, "Winston-ESP  v0.4-dev", BLACK, WHITE);

  // Content area. Show sensor data.
  char temp_hum_str[20];
  sprintf(temp_hum_str, "%.1fC/%.1fRH", temp_celsius_, rel_humidity_);
  oled_->draw_string(1, 16, temp_hum_str, WHITE, BLACK);

  // Show node name at the bottom of the content area.
  oled_->draw_string(1, 26, node_name_, WHITE, BLACK);

  // Bottom stats section, heap, network info, ...
  const int column_2_start = 6*4 + 2 + 1;
  const int col_3_start = 6*7 + 2 + 2;

  oled_->draw_hline(0, 36, oled_->get_width(), WHITE);

  // Free heap space display.
  oled_->fill_rectangle(0, 37, 6*4 + 1, 9, WHITE);
  oled_->draw_vline(0, 46, 9, WHITE);
  oled_->draw_vline(column_2_start - 1, 46, 9, WHITE);
  oled_->draw_string(1, 38, "HEAP", BLACK, WHITE);
  oled_->draw_string(2, 47, heap_free_msg_, WHITE, BLACK);

  // IP address / network status.
  oled_->fill_rectangle(column_2_start - 1, 37, 6*3 + 1, 9, WHITE);
  oled_->draw_string(column_2_start, 38, "CON", BLACK, WHITE);
  if (wifi_status_ == DISPLAY_WIFI_CONNECTED) {
    customSpacingText(oled_, col_3_start, 38, ip_address_, '.', 2, 1);
  } else {
    oled_->draw_string(col_3_start, 38,
                       getWifiString(wifi_status_), WHITE, BLACK);
  }

  // Print MAC address. Custom spacing to make it fit (thinner colons).
  oled_->fill_rectangle(column_2_start - 1, 46, 6*3 + 1, 9, WHITE);
  oled_->draw_string(column_2_start, 47, "MAC", BLACK, WHITE);
  customSpacingText(oled_, col_3_start, 47, mac_address_, ':', 2);

  oled_->draw_vline(oled_->get_width() - 1, 37, 18, WHITE);

  // Footer with time and date.
  oled_->fill_rectangle(0, 55, oled_->get_width(), 9, WHITE);
  oled_->draw_string(2, 56, date_time_, BLACK, WHITE);

  oled_->refresh(true);
  if (!this->locking_->unlockI2C(TAG)) {
    ESP_LOGE(TAG, "Cannot unlock I2C bus access.");
    return;
  }
}

void DisplayController::setNodeName(const std::string& node_name) {
  this->node_name_ = node_name;
}

void DisplayController::setWifiStatus(WifiStatus status) {
  this->wifi_status_ = status;
}

void DisplayController::setIpAddress(const std::string& address) {
  this->ip_address_ = address;
}

void DisplayController::setMacAddress(const std::string& mac) {
  this->mac_address_ = mac;
}

void DisplayController::setDateAndTime(const std::string& date_time) {
  this->date_time_ = date_time;
}

void DisplayController::setFreeHeapBytes(int free_bytes) {
  this->heap_free_msg_ = std::to_string(free_bytes / 1024) +  "K";
}

void DisplayController::setTemperature(float celsius) {
  this->temp_celsius_ = celsius;
}

void DisplayController::setHumidity(float rel_hum) {
  this->rel_humidity_ = rel_hum;
}
