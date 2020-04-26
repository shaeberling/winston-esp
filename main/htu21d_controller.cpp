#include "htu21d_controller.h"

#include <sstream>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG = "htu21d-ctrl";

static const int HTU21D_ADDR = 0x40;

HTU21DController::HTU21DController(const gpio_num_t scl, const gpio_num_t sda) :
    gpio_scl_(scl),
    gpio_sda_(sda),
    initialized_(false) {
}

bool HTU21DController::init() {
  if (initialized_) {
    ESP_LOGW(TAG, "HTU21D controller already initialized.");
    return false;
  }
  esp_err_t ret;
  
  // I2C basic setup.
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = this->gpio_sda_;
  conf.scl_io_num = this->gpio_scl_;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = 100000;
  ret = i2c_param_config(I2C_NUM_0, &conf);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to comfigure I2C for HTU21D.");
    return false;
  }
  
  ret = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
  if(ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to install I2C driver for HTU21D.");
    return false;
  }
  
  // Check if we can talk to the sensor.
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (HTU21D_ADDR << 1) | I2C_MASTER_WRITE, true);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "I2C Timeout talking to HTU21D.");
    return false;
  }
  ESP_LOGI(TAG, "Link to HTU21D established.");
  initialized_ = true;
  return true;
}


float HTU21DController::getCelsius() {
  if (!initialized_) {
    ESP_LOGE(TAG, "HTU21D controller not initialized.");
    return 0;
  }
  float result = getRaw();
  return result / 1000.0;
}

float HTU21DController::getHumidity() {
  if (!initialized_) {
    ESP_LOGE(TAG, "HTU21D controller not initialized.");
    return 0;
  }
  ESP_LOGW(TAG, "Warning, humidity not yet implemented.");
  return 23;
}

// private
int HTU21DController::getRaw() {
  // Sending command to request value.
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (HTU21D_ADDR << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, 0xF3, true);
  i2c_master_stop(cmd);
  esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Cannot write request command to HTU21D.");
    return 0;
  }

  // Give the sensor some time to respond, then receive the values requested.
  // Reading will fail, if this time is too short.
  vTaskDelay(100 / portTICK_RATE_MS);
  uint8_t msb, lsb, crc;
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (HTU21D_ADDR << 1) | I2C_MASTER_READ, true);
  i2c_master_read_byte(cmd, &msb, (i2c_ack_type_t) 0x00);
  i2c_master_read_byte(cmd, &lsb, (i2c_ack_type_t) 0x00);
  i2c_master_read_byte(cmd, &crc, (i2c_ack_type_t) 0x01);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Cannot read response from HTU21D.");
    return 0;
  }
  return ((msb << 8) + lsb) & 0xFFFC;
}