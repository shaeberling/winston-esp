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

ControlHub::ControlHub() {}

void ControlHub::registerController(Controller* controller) {
  // Note, we are owning the config pointers.
  auto sensors = controller->getSensors();
  for (auto& config : sensors) {
    registerSensor(config);
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
