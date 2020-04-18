/** UI controller for Winston ESP. */
#include "ui_controller.h"

#include <functional>
#include <sstream>
#include <string.h>

#include "events.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"

#include "oled_controller.h"

static const char *TAG = "win-ui-ctrl";

UiController::UiController(OledController* display)
    : display_(display), initiated_(false) {
}

void UiController::init() {
  if (initiated_) {
    ESP_LOGE(TAG, "UiController is already initiated.");
    return;
  }
  ESP_LOGI(TAG, "Enabling UI controller...");

  // Add events that modify the UI here.
  registerEvent(WIFI_EVENT, ESP_EVENT_ANY_ID);
  registerEvent(IP_EVENT, IP_EVENT_STA_GOT_IP);
  registerEvent(WINSTON_EVENT, WIFI_CONNECTED);
}

//non-static
void UiController::onEvent(esp_event_base_t event_base, 
                           int32_t event_id, void* event_data) {
  if (event_base == WINSTON_EVENT && event_id == WIFI_CONNECTED) {
    this->display_->setWifiStatus(DISPLAY_WIFI_CONNECTED);
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    this->display_->setWifiStatus(DISPLAY_WIFI_CONNECTING);
    this->display_->setIpAddress("...");
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    this->display_->setWifiStatus(DISPLAY_WIFI_NOT_CONNECTED);
    this->display_->setIpAddress("none");
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    char ip_str[16];  // enough to hold an IP address.
    sprintf(ip_str, IPSTR, IP2STR(&event->ip_info.ip));
    const std::string s(ip_str);
    this->display_->setIpAddress(s);
  } else {
    ESP_LOGI(TAG, "UI: Unandled event received.");
  }
}

void UiController::registerEvent(esp_event_base_t event_base, int32_t event_id) {
  ESP_ERROR_CHECK(
    esp_event_handler_register(event_base, event_id,
                               &UiController::event_handler, this));
}

// static
void UiController::event_handler(void* arg, esp_event_base_t event_base, 
                                 int32_t event_id, void* event_data) {
  static_cast<UiController*>(arg)->onEvent(event_base, event_id, event_data);
}