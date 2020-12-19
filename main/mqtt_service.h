#pragma once

#ifndef _WINSTON_MQTT_SERVICE_H_
#define _WINSTON_MQTT_SERVICE_H_

#include <string>

#include "mqtt_client.h"
#include "esp_event.h"

// A client for the MQTT service which we use to send out updates and receive
// control events.
class MqttService {
 public:
  // broker_url: The URL of the MQTT broker to connect to.
  // node_name : The name of this Winston node.
  MqttService(const std::string& broker_url, const std::string& node_name);

  // Call this once to initialize the service.
  // Returns whether initialization was successful.
  bool init();

  // Called for Winston sensor events from the ESP message bus.
  void onWinstonEvent(esp_event_base_t base, int32_t event_id, void* data);

  // Called for incoming MQTT messages.
  esp_err_t onMqttEvent(esp_mqtt_event_handle_t event);

 private:
  const std::string broker_url_;
  const std::string node_name_;

  // Reference to the latest MQTT client.
  esp_mqtt_client_handle_t client_;
};

#endif /* _WINSTON_MQTT_SERVICE_H_ */
