/* WiFi station Example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "events.h"
#include "hall_effect_controller.h"
#include "oled_controller.h"
#include "reed_controller.h"
#include "relay_controller.h"
#include "temp_controller.h"
#include "server.h"
#include "wifi.h"

#define WIFI_SSID   CONFIG_ESP_WIFI_SSID
#define WIFI_PASS   CONFIG_ESP_WIFI_PASSWORD
#define SERVER_PORT 80

static const char *TAG = "winston-main";

ESP_EVENT_DEFINE_BASE(WINSTON_EVENT);
namespace {

ReedController* reed_controller;
RelayController* relay_controller;
TempController* temp_controller;
HallEffectController* hall_controller;
OledController* oled_controller;

Server* server = NULL;
Wifi* wifi;

/** Called when WIFI connected (we have an IP). */
void onWifiConnected() {
  if (server != NULL) {
    ESP_LOGI(TAG, "Stopping previously started webserver...");
    server->stop();
    ESP_LOGI(TAG, "Webserver stopped.");
  }
  ESP_LOGI(TAG, "Wifi connected. Starting webserver ...");
  server = new Server(SERVER_PORT, reed_controller, relay_controller,
                      temp_controller, hall_controller);
  if (server->start()) {
    ESP_LOGI(TAG, "Webserver successfully started.");
  } else {
    ESP_LOGE(TAG, "Starting the webserver failed.");
  }
}

void event_handler(void* arg, esp_event_base_t event_base, 
                   int32_t event_id, void* event_data) {
  if (event_base == WINSTON_EVENT && event_id == WIFI_CONNECTED) {
    onWifiConnected();
  } else {
    ESP_LOGW(TAG, "Received unknown winston event.");
  }
}

// Initialize NVS
void initNvs() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

}  // namespace


extern "C" {

void app_main(void) {
  // Can control an external OLED display.
  // TODO: Add an sdkconfig variable about activating it or not (same for other modules).
  oled_controller = new OledController();

  ESP_LOGI(TAG, ".: Winston ESP Node :.");

  // Default loop required for various events.
  // (We could create our own event loops, but for now using the default seems fine).
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(esp_event_handler_register(WINSTON_EVENT, WIFI_CONNECTED,
                                             &event_handler, NULL));

  // TODO: Make these configurable through flags.
  // Note: GPIO-5 should not be used for the relay (outputs PWM on startup).
  std::vector<int> reed_mapping = { 5, 18 };
  reed_controller = new ReedController(reed_mapping);
  std::vector<int> relay_mapping = { 19, 21 };
  relay_controller = new RelayController(relay_mapping);
  temp_controller = new TempController();
  hall_controller = new HallEffectController();

  initNvs();
  ESP_LOGI(TAG, "NVS initialized. Connecting to Wifi...");
  wifi = new Wifi;
  wifi->connect(WIFI_SSID, WIFI_PASS);

  // Note: This will change ADC config and will use up some pins around 36.
  hall_controller->init();
}

} // extern "C"
