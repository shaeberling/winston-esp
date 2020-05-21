#include <memory>
#include <string>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "display_controller.h"
#include "events.h"
#include "hall_effect_controller.h"
#include "htu21d_controller.h"
#include "locking.h"
#include "mongoose_server.h"
#include "reed_controller.h"
#include "relay_controller.h"
#include "request_handler.h"
#include "server.h"
#include "temp_controller.h"
#include "time_controller.h"
#include "ui_controller.h"
#include "wifi.h"

#define WIFI_SSID   CONFIG_WINSTON_WIFI_SSID
#define WIFI_PASS   CONFIG_WINSTON_WIFI_PASSWORD
#define TIMEZONE    CONFIG_WINSTON_TIMEZONE

#define SERVER_PORT 80
#define USE_MONGOOSE true

static const char *TAG = "winston-main";

ESP_EVENT_DEFINE_BASE(WINSTON_EVENT);
namespace {

Locking* locking;
ReedController* reed_controller;
RelayController* relay_controller;
TempController* temp_controller;
HallEffectController* hall_controller;
DisplayController* display_controller;
UiController* ui_controller;
HTU21DController* htu21d_controller;
TimeController* time_controller;
SystemController* system_controller;
RequestHandler* request_handler;

std::unique_ptr<Server> server;
std::unique_ptr<MongooseServer> mg_server;
Wifi* wifi;

/** Called when WIFI connected (we have an IP). */
void onWifiConnected() {
  // Note: "server" is using the ESP built-in httpd implementation. We had a few
  // issues with this. At randmon times, it seems to stop responding when being
  // queried from a mobile Chrome browser.
  // We therefore implemented an alternative using mongoose to see if this
  // eleviates the issue.
  // During this evaluation, both implementations are available here.
  if (server) {
    ESP_LOGI(TAG, "Stopping previously started webserver...");
    server->stop();
    ESP_LOGI(TAG, "Webserver stopped.");
  }
  if (mg_server) {
    ESP_LOGI(TAG, "Stopping previously started mongoose server...");
    mg_server->stop();
    ESP_LOGI(TAG, "Mongoose server stopped.");
  }

  ESP_LOGI(TAG, "Wifi connected. Starting webserver ...");
  if (USE_MONGOOSE) {
    mg_server.reset(new MongooseServer(SERVER_PORT, request_handler));
  } else {
    server.reset(new Server(SERVER_PORT, request_handler));
  }

  if (USE_MONGOOSE) {
    if (mg_server->start()) {
      ESP_LOGI(TAG, "Mongoose server successfully started.");
    } else {
      ESP_LOGE(TAG, "Starting the Mongoose server failed.");
    }
  } else {
    if (server->start()) {
      ESP_LOGI(TAG, "Webserver successfully started.");
    } else {
      ESP_LOGE(TAG, "Starting the webserver failed.");
    }
  }

  ESP_LOGI(TAG, "Triggering NTP sync");
  time_controller->syncWithNtp();
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
  ESP_LOGI(TAG, ".: Winston ESP Node :.");

  // Default loop required for various events.
  // (We could create our own event loops, but for now using the default seems fine).
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(esp_event_handler_register(WINSTON_EVENT, WIFI_CONNECTED,
                                             &event_handler, NULL));

  locking = new Locking();

  // TODO: Make these configurable through flags.
  // Note: GPIO-5 should not be used for the relay (outputs PWM on startup).
  // See usable GPIOs here:
  // https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
  std::vector<int> reed_mapping = { 14, 27 };
  reed_controller = new ReedController(reed_mapping);
  std::vector<int> relay_mapping = { 26, 25 };
  relay_controller = new RelayController(relay_mapping);
  htu21d_controller = new HTU21DController(GPIO_NUM_22, GPIO_NUM_21, locking);
  temp_controller = new TempController(htu21d_controller);
  hall_controller = new HallEffectController();
  display_controller = new DisplayController(GPIO_NUM_22, GPIO_NUM_21, locking);
  time_controller = new TimeController(TIMEZONE);
  system_controller = new SystemController();
  ui_controller = new UiController(display_controller,
                                   time_controller,
                                   system_controller);
  request_handler = new RequestHandler(reed_controller, relay_controller,
                                       temp_controller, hall_controller,
                                       time_controller, system_controller);

  initNvs();
  ESP_LOGI(TAG, "NVS initialized. Connecting to Wifi...");
  wifi = new Wifi;
  wifi->connect(WIFI_SSID, WIFI_PASS);

  // Note: This will change ADC config and will use up some pins around 36.
  // TODO: Make this a start-up config parameter.
  // hall_controller->init();
  // TODO: Add an sdkconfig variable about activating it or not (same for other modules).
  display_controller->init();
  ui_controller->init();
  // htu21d_controller->init();
}

} // extern "C"
