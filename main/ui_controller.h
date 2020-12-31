#pragma once

#ifndef _WINSTON_UI_CONTROLLER_H_
#define _WINSTON_UI_CONTROLLER_H_

#include <string>
#include <vector>

#include "esp_event.h"

#include "controller.h"
#include "display_controller.h"
#include "system_controller.h"
#include "time_controller.h"

class UiController : public Controller {
 public:
  UiController(DisplayController* display,
               TimeController* time_controller,
               SystemController* system_controller);
  // Enables listening to events to update display contents.
  bool init() override;
  std::vector<SensorConfig*> getSensors() override;
 private:
  DisplayController* display_;
  TimeController* time_;
  SystemController* system_;

  bool initiated_;
  int connection_attempts_;

  void onEvent(esp_event_base_t event_base, int32_t event_id,
               void* event_data);

  void registerEvent(esp_event_base_t event_base, int32_t event_id);
  static void event_handler(void* arg, esp_event_base_t event_base, 
                            int32_t event_id, void* event_data);
  void onUpdateUi();
  static void startUpdateLoop(void* p);
};

#endif /* _WINSTON_UI_CONTROLLER_H_ */
