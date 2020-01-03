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
};

#ifdef __cplusplus
}
#endif

#endif /* _WINSTON_EVENTS_H_ */