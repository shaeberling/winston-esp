#pragma once

#ifndef _WINSTON_BME280_CONTROLLER_H_
#define _WINSTON_BME280_CONTROLLER_H_

#include "bme280.h"

#include "controller.h"
#include "locking.h"
#include "driver/gpio.h"

#include <string>
#include <vector>

/** Controller for the Bosch BME280 (not BMP280!). */
class BME280Controller : public Controller {
 public:
  BME280Controller(const std::string& id,
                   const gpio_num_t scl,
                   const gpio_num_t sda,
                   Locking* locking);

  bool init() override;
  void registerIO(std::vector<SensorConfig*>*,
                  std::vector<ActuatorConfig*>*) override;

  float getCelsius() /*const*/;
  float getHumidity() /*const*/;
  float getPressure() /*const*/;
 private:
  const std::string id_;
  const gpio_num_t gpio_scl_;
  const gpio_num_t gpio_sda_;
  Locking* locking_;
  BME280 bme280_;
  bool initialized_;

  bool initInternal();
  bme280_reading_data getUpdateInternal();
};

#endif /* _WINSTON_BME280_CONTROLLER_H_ */
