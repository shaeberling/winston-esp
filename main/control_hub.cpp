#include "control_hub.h"

#include "controller.h"
#include "events.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "time_controller.h"

#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>

static const char *TAG = "win-ctrl-hub";

namespace {

struct TaskConfig {
  SensorConfig* config;
  ControlHub* hub;
};

}  // namespace

ControlHub::ControlHub() {
    esp_event_handler_register(WINSTON_EVENT, ACTUATOR_EVENT,
                               &ControlHub::actuator_event_handler, this);
}

void ControlHub::handleActuatorEvent(esp_event_base_t event_base, 
                                     int32_t event_id, void* event_data) {
  if (event_base != WINSTON_EVENT || event_id != ACTUATOR_EVENT) {
    return;
  }
  auto* update = static_cast<SensorUpdate*>(event_data);
  auto pos = actuators_.find(update->sensor_path);
  if (pos == actuators_.end()) {
    ESP_LOGE(TAG, "Cannot find actuator '%s'", update->sensor_path.c_str());
  } else {
    pos->second(update->value_str);
  }
}

// static
void ControlHub::actuator_event_handler(void* arg, esp_event_base_t event_base, 
                                        int32_t event_id, void* event_data) {
  static_cast<ControlHub*>(arg)->handleActuatorEvent(event_base,
                                                     event_id,
                                                     event_data);
}

void ControlHub::registerController(Controller* controller) {
  std::vector<SensorConfig*> sensors;
  std::vector<ActuatorConfig*> actuators;
  controller->registerIO(&sensors, &actuators);
  // Note, we are owning the config pointers.
  for (auto* config : sensors) {
    registerSensor(config);
  }
  for (auto* config : actuators) {
    registerActuator(config);
  }
}

void ControlHub::registerSensor(SensorConfig* config) {
  std::string task_name = "sensor_updater_" + config->name + "_" + config->id;
  ESP_LOGI(TAG, "Registering new sensor: %s", task_name.c_str());

  // Needs to be on the heap, not stack.
  TaskConfig* task = new TaskConfig {
    .config = config,
    .hub = this
  };

  // TODO: We can turn this into a single task that wakes up every second and
  //       then fires updates for the sensors when their timer is up.
  //       This would probably safe us a lot of stack size.
  auto rt = xTaskCreatePinnedToCore(
      ControlHub::startUpdateLoop, task_name.c_str(), 2000, (void*) task, 1, NULL, 0);
  if (rt != pdPASS) {
    ESP_LOGE(TAG, "Cannot create update task for %s", task_name.c_str());
  }
}

void ControlHub::registerActuator(ActuatorConfig* config) {
  std::string rel_path = config->name + "/" + config->id;
  actuators_.insert(
      std::pair<std::string, std::function<bool(const std::string&)>>(
          rel_path, config->set_value)); 
}

// private
void ControlHub::onSensorUpdate(const std::string& path, const std::string& value) {
  auto sensor_event = SensorUpdate {
    .sensor_path = path,
    .value_str = value
  };
  // Note: This will make a copy of the event payload data.
  esp_event_post(WINSTON_EVENT, SENSOR_EVENT, &sensor_event,
                 sizeof(sensor_event), portMAX_DELAY);
}

// private static
void ControlHub::startUpdateLoop(void* param) {
  auto* task = static_cast<TaskConfig*>(param);

  const auto* config = task->config;
  auto path = config->name + "/" + config->id;

  // We don't want to or need to stop a loop of a sensor right now.
  while(true) {
    task->hub->onSensorUpdate(path, config->get_value());
    vTaskDelay(config->update_interval_seconds * 1000 / portTICK_PERIOD_MS);
  }
  // elete task;
}
