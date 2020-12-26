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

#include "display_controller.h"
#include "system_controller.h"
#include "time_controller.h"

static const char *TAG = "win-ui-ctrl";

// Update non-event-driven UI items once very 5 seconds.
static const int UI_UPDATE_DELAY_MILLIS = 5000;

UiController::UiController(DisplayController* display,
                           TimeController* time,
                           SystemController* system)
    : display_(display),
      time_(time),
      system_(system),
      initiated_(false),
      connection_attempts_(0) {
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
  registerEvent(WINSTON_EVENT, SENSOR_EVENT);

  auto rt = xTaskCreatePinnedToCore(
     UiController::startUpdateLoop, "ui-updater", 5000, this, 1, NULL, 0);
  if (rt != pdPASS) {
   ESP_LOGE(TAG, "Cannot create UI update task.");
  }

  // The MAC address never changes, so set it only once.
  display_->setMacAddress(system_->getMacAddress());
}

// private
void UiController::onEvent(esp_event_base_t event_base, 
                           int32_t event_id, void* event_data) {
  if (event_base == WINSTON_EVENT && event_id == SENSOR_EVENT) {
    auto* update = static_cast<SensorUpdate*>(event_data);

    // 0 is the inaccurate internal sensor.
    if (update->sensor_path == "temp/1") {
      this->display_->setTemperature(std::stof(update->value_str));
    } else if (update->sensor_path == "hum/0") {
      this->display_->setHumidity(std::stof(update->value_str));
    }
  } else if (event_base == WINSTON_EVENT && event_id == WIFI_CONNECTED) {
    this->connection_attempts_ = 0;
    this->display_->setWifiStatus(DISPLAY_WIFI_CONNECTED);
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    this->display_->setWifiStatus(DISPLAY_WIFI_CONNECTING);
    this->display_->setIpAddress("...");
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    this->connection_attempts_++;
    this->display_->setWifiStatus(DISPLAY_WIFI_NOT_CONNECTED);
    // When disconnected, we will retry until WIFI_CONNECTED. Thus, show
    // a useful message about how many re-connect attemps we've tried.
    std::string ip_message("Attempts: ");
    ip_message += std::to_string(this->connection_attempts_);
    this->display_->setIpAddress(ip_message);
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

// private
void UiController::registerEvent(esp_event_base_t event_base, int32_t event_id) {
  ESP_ERROR_CHECK(
    esp_event_handler_register(event_base, event_id,
                               &UiController::event_handler, this));
}

// private static
void UiController::event_handler(void* arg, esp_event_base_t event_base, 
                                 int32_t event_id, void* event_data) {
  static_cast<UiController*>(arg)->onEvent(event_base, event_id, event_data);
}

// private
void UiController::onUpdateUi() {
  // Update things on a regular basis here.
  // Only fetch things here for which we don't get regular events.
  // Then ensure the display is updated with the new information.
  this->display_->setDateAndTime(time_->getDateAndTime(6));
  this->display_->setFreeHeapBytes(system_->getFreeHeapBytes());
  this->display_->update();
}
// private static
void UiController::startUpdateLoop(void* p) {
  while (true) {
    static_cast<UiController*>(p)->onUpdateUi();
    vTaskDelay(UI_UPDATE_DELAY_MILLIS / portTICK_PERIOD_MS);
  }
}
