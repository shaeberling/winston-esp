#include "locking.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Can be quite verbose when many display updates happen.
#define ENABLE_LOGGING false

static const char *TAG = "winston-locking";
static const int TAKE_DELAY_MILLIS = 10000;

Locking::Locking()
    : i2c_handle_(xSemaphoreCreateMutex()) {
}

// public
bool Locking::lockI2C(const char* who) {
  if (this->i2c_handle_ == NULL) {
    ESP_LOGE(TAG, "[%s] Semaphore not created successfully", who);
    return false;
  }
  if (ENABLE_LOGGING) ESP_LOGI(TAG, "[%s] Attemping to lock I2C", who);
  return xSemaphoreTake(this->i2c_handle_,
                        TAKE_DELAY_MILLIS / portTICK_PERIOD_MS) == pdTRUE;
}

bool Locking::unlockI2C(const char* who) {
  if (this->i2c_handle_ == NULL) {
    ESP_LOGE(TAG, "[%s] Semaphore not created successfully", who);
    return false;
  }
  if (ENABLE_LOGGING) ESP_LOGI(TAG, "[%s] Unlocking I2C", who);

  // With a little delay to space out i2c operations.
  vTaskDelay(100 / portTICK_PERIOD_MS);
  return xSemaphoreGive(this->i2c_handle_) == pdTRUE;
}
