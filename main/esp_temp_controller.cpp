#include "esp_temp_controller.h"

#include <string>
#include <vector>

#include "esp_log.h"

#include "controller.h"

#ifdef __cplusplus
extern "C" {
#endif
extern uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif

EspTempController::EspTempController(const std::string& id)
    : id_(id) {
}

bool EspTempController::init() {
  return true;
}

// override
void EspTempController::registerIO(std::vector<SensorConfig*>* sensors,
                                   std::vector<ActuatorConfig*>* actuators) {
  sensors->push_back(new SensorConfig {
    .name = "temp",
    .id = this->id_,
    .update_interval_seconds = 10,
    .get_value = [](void){ return std::to_string(temprature_sens_read() - 32); }
  });
}