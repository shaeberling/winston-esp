#include "mqtt_service.h"


#include <iterator>
#include <sstream>
#include <string.h>
#include <vector>

#include "esp_event.h"
#include "esp_log.h"
#include "events.h"
#include "mqtt_client.h"


static const char *TAG = "win-mqtt";


namespace {

// Forwards C-style functional ESP events to the MqttService class.
static void event_handler(void* arg, esp_event_base_t base, 
                          int32_t event_id, void* data) {
  static_cast<MqttService*>(arg)->onWinstonEvent(base, event_id, data);
}

// Forwards C-style functional MQTT events to the MqttService class.
static void mqtt_event_handler(void *arg,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data) {
  ESP_LOGI(TAG, "Event dispatched: event_base=%s, event_id=%d", base, event_id);
  static_cast<MqttService*>(arg)->onMqttEvent(
      static_cast<esp_mqtt_event_handle_t>(event_data));
}

void split(const std::string& str,
           const char delim,
           std::vector<std::string>* items) {
  std::stringstream ss(str);
  std::string token;
  while (std::getline(ss, token, delim)) {
    items->push_back(token);
  }
}

}  // namespace


MqttService::MqttService(const std::string& broker_url,
                         const std::string& node_name)
    : broker_url_(broker_url),
      node_name_(node_name) {
}

// public 
bool MqttService::init() {
  esp_mqtt_client_config_t mqtt_cfg = {
      .uri = broker_url_.c_str(),
  };

  client_ = esp_mqtt_client_init(&mqtt_cfg);
  // FIXME: Make the data be "this" and route actions like open garage that way.
  esp_mqtt_client_register_event(client_, MQTT_EVENT_ANY, mqtt_event_handler,
                                 this);
  esp_mqtt_client_start(client_);

  ESP_ERROR_CHECK(esp_event_handler_register(WINSTON_EVENT, ESP_EVENT_ANY_ID,
                                             &event_handler, this));
  ESP_LOGI(TAG, "MQTT service started. Listening to Winston events.");
  return true;
}

// Receives sensor events on the ESP event bus and broadcasts them to MQTT.
void MqttService::onWinstonEvent(esp_event_base_t base, 
                                 int32_t event_id, void* data) {
  if (base != WINSTON_EVENT || event_id != SENSOR_EVENT) {
    return;
  }
  auto sensor_data = static_cast<SensorUpdate*>(data);

  // The Winston MQTT schema: "winston/<node>/path"
  std::ostringstream topic;
  topic << "winston/" << node_name_ << "/" << sensor_data->sensor_path;
  ESP_LOGI(TAG, "Posting to topic: '%s'", topic.str().c_str());
  esp_mqtt_client_publish(client_, topic.str().c_str(),
                          sensor_data->value_str.c_str(), 0, 0, 0);
}

// Receives events from the MQTT service.
esp_err_t MqttService::onMqttEvent(esp_mqtt_event_handle_t event) {
  esp_mqtt_client_handle_t client = event->client;
  switch (event->event_id) {
      case MQTT_EVENT_CONNECTED: {
          ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
          std::ostringstream topic;
          topic << "winston/" << node_name_ << "/in/#";
          auto msg_id = esp_mqtt_client_subscribe(client, topic.str().c_str(), 0);
          if (msg_id != -1) {
            ESP_LOGI(TAG, "Successfully subscribed to node updates.");
          } else {
            ESP_LOGE(TAG, "Unable to subscribe to node updates.");
          }
          break;
      }
      case MQTT_EVENT_DISCONNECTED:
          ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
          break;
      case MQTT_EVENT_SUBSCRIBED:
          ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
          break;
      case MQTT_EVENT_UNSUBSCRIBED:
          ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
          break;
      case MQTT_EVENT_PUBLISHED:
          ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
          break;
      case MQTT_EVENT_DATA: {
          ESP_LOGI(TAG, "MQTT_EVENT_DATA");
          char buffer [30];
          sprintf(buffer, "%.*s", event->topic_len, event->topic);
          std::string topic = buffer;
          sprintf(buffer, "%.*s", event->data_len, event->data);
          std::string data = buffer;
          ESP_LOGI(TAG, "Topic: %s, Data: %s", topic.c_str(), data.c_str());

          std::vector<std::string> path;
          split(topic, '/', &path);

          if (path[0] != "winston" ||
              path[1] != node_name_ ||
              path[2] != "in" ||
              path.size() != 5) {
            ESP_LOGE(TAG, "Internal error: Bad path: '%s'", topic.c_str());
            return ESP_ERR_INVALID_ARG;
          }
          // Delete the first two elements to turn path into relative path.
          std::string rel_path = path[3] + "/" + path[4];
          ESP_LOGI(TAG, "Rel Path: '%s' -> '%s'", rel_path.c_str(), data.c_str());

          auto actuator_event = SensorUpdate {
            .sensor_path = rel_path,
            .value_str = data
          };
          // Note: This will make a copy of the event payload data.
          esp_event_post(WINSTON_EVENT, ACTUATOR_EVENT, &actuator_event,
                         sizeof(actuator_event), portMAX_DELAY);
          break;
      }
      case MQTT_EVENT_ERROR:
          ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
          break;
      default:
          ESP_LOGI(TAG, "Other event id:%d", event->event_id);
          break;
  }
  return ESP_OK;
}
