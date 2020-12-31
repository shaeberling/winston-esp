#include "htu21d_controller.h"

#include "controller.h"
#include "htu21d_controller.h"
#include "locking.h"

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"

#include <sstream>
#include <string>
#include <vector>

static const char* TAG = "htu21d-ctrl";

static const int HTU21D_ADDR = 0x40;


static const int TEMP_NO_HOLD_MASTER = 0xF3;
static const int HUMI_NO_HOLD_MASTER = 0xF5;

HTU21DController::HTU21DController(const std::string& id,
                                   const gpio_num_t scl,
                                   const gpio_num_t sda,
                                   Locking* locking) :
    id_(id),
    gpio_scl_(scl),
    gpio_sda_(sda),
    locking_(locking),
    initialized_(false) {
}

// override
bool HTU21DController::init() {
  if (initialized_) {
    ESP_LOGW(TAG, "HTU21D controller already initialized.");
    return false;
  }
  esp_err_t ret;

  if (!this->locking_->lockI2C(TAG)) {
    ESP_LOGE(TAG, "Cannot lock I2C bus access.");
    return false;
  }

  // I2C basic setup.
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = this->gpio_sda_;
  conf.scl_io_num = this->gpio_scl_;
  conf.sda_pullup_en = GPIO_PULLUP_ONLY;
  conf.scl_pullup_en = GPIO_PULLUP_ONLY;
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

  if (!this->locking_->unlockI2C(TAG)) {
    ESP_LOGE(TAG, "Cannot unlock I2C bus access.");
    return false;
  }
  initialized_ = true;
  return true;
}

// override
std::vector<SensorConfig*> HTU21DController::getSensors() const {
  std::vector<SensorConfig*> sensors;

  sensors.push_back(new SensorConfig {
    .name = "temp",
    .id = this->id_,
    .update_interval_seconds = 10,
    .get_value = [this](void){ return std::to_string(this->getCelsius()); }
  });

  sensors.push_back(new SensorConfig {
    .name = "hum",
    .id = this->id_,
    .update_interval_seconds = 10,
    .get_value = [this](void){ return std::to_string(this->getHumidity()); }
  });

  return sensors;
}


float HTU21DController::getCelsius() const {
  if (!initialized_) {
    ESP_LOGE(TAG, "HTU21D controller not initialized.");
    return 0;
  }
  // See page 15 of the htu21d_datasheet.pdf for details on this:
  float result = getRawWithLock(TEMP_NO_HOLD_MASTER);
  result *= 175.72;
  result /= (2 << 15);
  result -= 46.85;
  return result;
}

float HTU21DController::getHumidity() const {
  if (!initialized_) {
    ESP_LOGE(TAG, "HTU21D controller not initialized.");
    return 0;
  }
  // See page 15 of the htu21d_datasheet.pdf for details on this:
  float result = getRawWithLock(HUMI_NO_HOLD_MASTER);
  result *= 125;
  result /= (2 << 15);
  result -= 6;
  return result;
}

// private
int HTU21DController::getRawWithLock(int command) const {
  if (!this->locking_->lockI2C(TAG)) {
    ESP_LOGE(TAG, "Cannot lock I2C bus access.");
    return 0;
  }
  int value = this->getRaw(command);
    if (!this->locking_->unlockI2C(TAG)) {
    ESP_LOGE(TAG, "Cannot unlock I2C bus access.");
    return 0;
  }
  return value;
}

int HTU21DController::getRaw(int command) const {
  esp_err_t ret;
  // Sending command to request value.
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (HTU21D_ADDR << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, command, true);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
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

  // TODO: Use CRC to ensure result is correct. (See specsheet p.14).
  return ((msb << 8) + lsb) & 0xFFFC;
}