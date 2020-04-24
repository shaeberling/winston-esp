#pragma once

#ifndef _WINSTON_UI_CONTROLLER_H_
#define _WINSTON_UI_CONTROLLER_H_

#include <string>

#include "esp_event.h"

#include "display_controller.h"

class UiController {
 public:
  UiController(DisplayController* display);
  // Enables listening to events to update display contents.
  void init();
 private:
  DisplayController* display_;
  bool initiated_;
  int connection_attempts_;
  void onEvent(esp_event_base_t event_base, int32_t event_id,
               void* event_data);

  void registerEvent(esp_event_base_t event_base, int32_t event_id);
  static void event_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data);
};

#endif /* _WINSTON_UI_CONTROLLER_H_ */
