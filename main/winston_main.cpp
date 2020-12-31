#include <memory>
#include <string>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "control_hub.h"
#include "controller_factory.h"
#include "events.h"
#include "locking.h"
#include "mongoose_server.h"
#include "mqtt_service.h"
#include "request_handler.h"
#include "server.h"
#include "settings_loader.h"
#include "time_controller.h"
#include "ui_controller.h"
#include "wifi.h"

#define WIFI_SSID    CONFIG_WINSTON_WIFI_SSID
#define WIFI_PASS    CONFIG_WINSTON_WIFI_PASSWORD

#define SERVER_PORT 80
#define USE_MONGOOSE true

#define ESP_INTR_FLAG_DEFAULT 0

static const char *TAG = "winston-main";

ESP_EVENT_DEFINE_BASE(WINSTON_EVENT);

namespace {

Locking* locking;
ControlHub* control_hub;
TimeController* time_controller;
std::vector<Controller*> controllers;

MqttService* mqtt;
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

  // TODO: Might deprecate this way of getting values in favor of MQTT.
  //       Will revive the server to start settings.
  /*ESP_LOGI(TAG, "Wifi connected. Starting webserver ...");
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
  }*/

  ESP_LOGI(TAG, "Triggering NTP sync");
  time_controller->syncWithNtp();

  ESP_LOGI(TAG, "Initializing MQTT");
  mqtt->init();
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

  // Load settings.
  Settings settings = {
    .device_settings = DeviceSettingsProto_init_zero,
    {}
  };

  SettingsLoader settings_loader;
  bool success = settings_loader.loadSettings(&settings);
  if (!success) {
    ESP_LOGE(TAG, "Cannot load embedded config. Aborting.");
    abort();
  }
  const auto& device_settings = settings.device_settings;

  ESP_LOGI(TAG, "Read embedded config. Node is '%s'", device_settings.node_name);
  ESP_LOGI(TAG, "mqtt_server_url: '%s'", device_settings.mqtt_server_url);
  ESP_LOGI(TAG, "time_zone: '%s'", device_settings.time_zone);

  // Default loop required for various events.
  // (We could create our own event loops, but for now using the default seems fine).
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(esp_event_handler_register(WINSTON_EVENT, WIFI_CONNECTED,
                                             &event_handler, NULL));

  // Install the driver’s GPIO ISR handler service, which allows per-pin GPIO
  // interrupt handlers.
  // This is e.g. used by the PIR sensor.
  ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT));

  locking = new Locking();

  // System controller is not configured and always present.
  auto* system_controller = new SystemController(device_settings.node_name);
  time_controller = new TimeController(device_settings.time_zone);

  controllers.push_back(system_controller);
  ControllerFactory factory(locking, time_controller, system_controller);
  for (auto& component : settings.components) {
    auto* controller = factory.createController(component);
    if (controller != NULL) controllers.push_back(controller);
  }

  // TODO: Add a sanity check before initializing things. Ensure no pins are
  //       used twice and ensure we don't use any that are not that usable.
  // Note: GPIO-5 should not be used for the relay (outputs PWM on startup).
  // See usable GPIOs here:
  // https://randomnerdtutorials.com/esp32-pinout-reference-gpios/


  control_hub = new ControlHub();
  mqtt = new MqttService(device_settings.mqtt_server_url, device_settings.node_name);

  initNvs();

  ESP_LOGI(TAG, "NVS initialized. Connecting to Wifi...");
  wifi = new Wifi;
  wifi->connect(WIFI_SSID, WIFI_PASS);

  // Note: This will change ADC config and will use up some pins around 36.
  // TODO: Make this a start-up config parameter.
  // hall_controller->init();
  // pir_controller->init(); // Note: Needs interrupts.

  for(auto* controller : controllers) {
    controller->init();
    control_hub->registerController(controller);
  }
}

} // extern "C"
