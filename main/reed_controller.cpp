#include "reed_controller.h"

#include "esp_log.h"

static const char *TAG = "reed-controller";

// public 
bool ReedController::status(int idx) {
  ESP_LOGI(TAG, "Getting reed controller status for %d", idx);
  return false;
}