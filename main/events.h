#pragma once

#ifndef _WINSTON_EVENTS_H_
#define _WINSTON_EVENTS_H_

#include "esp_event.h"

#ifdef __cplusplus
extern "C" {
#endif

ESP_EVENT_DECLARE_BASE(WINSTON_EVENT);

enum {
    WIFI_CONNECTED,
    SENSOR_EVENT,
};

struct SensorUpdate {
  // E.g. "/temp/1" or "/reed/0"
  const std::string sensor_path;
  // The value as a string. If this represents a number, it needs to be
  // converted.
  const std::string value_str;
};

#ifdef __cplusplus
}
#endif

#endif /* _WINSTON_EVENTS_H_ */