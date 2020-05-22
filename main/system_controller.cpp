#include "system_controller.h"

#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

SystemController::SystemController() {
}

int SystemController::getFreeHeapBytes() {
  return xPortGetFreeHeapSize();
}

std::string SystemController::getRunTimeStats() {
  char buffer[40 * 20];
  vTaskGetRunTimeStats(buffer);
  return std::string(buffer);
}
