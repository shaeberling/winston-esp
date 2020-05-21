#include "system_controller.h"

#include "freertos/FreeRTOS.h"

SystemController::SystemController() {
}

int SystemController::getFreeHeapBytes() {
  return xPortGetFreeHeapSize();
}
