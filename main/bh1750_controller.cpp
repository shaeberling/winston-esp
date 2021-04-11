#include "bh1750_controller.h"

#include "controller.h"
#include "locking.h"

#include "bh1750.h"
#include "esp_log.h"

#include <sstream>
#include <string>
#include <vector>

static const char* TAG = "bh1750-ctrl";

static const int HTU21D_ADDR = 0x23;

BH1750Controller::BH1750Controller(const std::string& id,
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
bool BH1750Controller::init() {
  if (initialized_) {
    ESP_LOGW(TAG, "BH1750 controller already initialized.");
    return false;
  }
  if (!this->locking_->lockI2C(TAG)) {
    ESP_LOGE(TAG, "Cannot lock I2C bus access.");
    return false;
  }


  // TODO

  initialized_ = true;
  return true;
}

// override
void BH1750Controller::registerIO(std::vector<SensorConfig*>* sensors,
                                  std::vector<ActuatorConfig*>* actuators) {
  sensors->push_back(new SensorConfig {
    .name = "light",
    .id = this->id_,
    .update_interval_seconds = 10,
    .get_value = [this](void){ return std::to_string(this->getLux()); }
  });
}


float BH1750Controller::getLux() const {
  if (!initialized_) {
    ESP_LOGE(TAG, "HTU21D controller not initialized.");
    return 99.0f;
  }

  // TODO
  return 42.0f;
}
