#include "oled_controller.h"

#include "esp_log.h"
#include "fonts.h"
#include "ssd1306.hpp"

static const char *TAG = "hall-controller";

// Note: the display has 16 pixels of yellow, a little gap and 48 blue lines. 
OledController::OledController() {
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

  // Just testing....
  if (!active_) {
    return;
  }
  oled_->clear();
  oled_->select_font(1);
  oled_->fill_rectangle(0, 0, oled_->get_width(), 16, WHITE);
  oled_->draw_string(23, 4, "Winston               v0.1", BLACK, WHITE);
  oled_->draw_string(11, 40, "All systems good :-)", WHITE, BLACK);
  oled_->refresh(true);
}
