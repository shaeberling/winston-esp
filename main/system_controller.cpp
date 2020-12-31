#include "system_controller.h"

#include <sstream>
#include <string>

#include "controller.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "sdkconfig.h"

static const char *TAG = "win-system";

SystemController::SystemController(const std::string& node_name)
        : node_name_(node_name) {}

// override
bool SystemController::init() {
  return true;
}

// override
std::vector<SensorConfig*> SystemController::getSensors() {
  std::vector<SensorConfig*> sensors;

  auto* c = new SensorConfig {
    .name = "system",
    .id = "heap",
    .update_interval_seconds = 60,
    .get_value = [this](void){ return std::to_string(this->getFreeHeapBytes()); }
  };
  sensors.push_back(c);

  return sensors;
}

int SystemController::getFreeHeapBytes() const {
  return esp_get_free_heap_size();
}

std::string SystemController::getRunTimeStats() const {
  char buffer[40 * 20];
  vTaskGetRunTimeStats(buffer);
  return std::string(buffer);
}

bool SystemController::restart(int delay_millis) {
  int* d_millis = new int;
  *d_millis = delay_millis;
  auto rt = xTaskCreatePinnedToCore(
     [](void* p){
       int millis = *(static_cast<int*>(p));
       ESP_LOGI(TAG, "Restarting in %d milliseconds.", millis);
       vTaskDelay(millis / portTICK_PERIOD_MS);
       esp_restart();
     }, "restarter", 5000, d_millis, 1, NULL, 0);
  return rt == pdPASS;
}

std::string SystemController::getSystemInfo() const {
  esp_chip_info_t info;
  esp_chip_info(&info);

  std::ostringstream output;

  output << "Chip     : " << CONFIG_IDF_TARGET << "\n";
  output << "Revision : " << (int) info.revision << "\n";
  output << "CPU Cores: " << (int) info.cores << "\n";
  output << "Flash    : " << spi_flash_get_chip_size() / (1024 * 1024) << "MB ("
         << ((info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external")
         << ")\n";
  output << "Bluetooth: " << ((info.features & CHIP_FEATURE_BT) ? "Yes" : "No")
         << "\n";
  output << "BLE      : " << ((info.features & CHIP_FEATURE_BLE) ? "Yes" : "No")
         << "\n";
  return output.str();
}

std::string SystemController::getMacAddress() const {
  uint8_t mac[6];
  esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
  char buf[20];
  sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return buf;
}

std::string SystemController::getNodeName() const {
  return node_name_;
}