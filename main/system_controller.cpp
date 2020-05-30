#include "system_controller.h"

#include <sstream>
#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "sdkconfig.h"

static const char *TAG = "win-system";

SystemController::SystemController() {
}

int SystemController::getFreeHeapBytes() {
  return esp_get_free_heap_size();
}

std::string SystemController::getRunTimeStats() {
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

std::string SystemController::getSystemInfo() {
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
