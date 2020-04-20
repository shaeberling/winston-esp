#pragma once

#ifndef _WINSTON_RELAY_CONTROLLER_H_
#define _WINSTON_RELAY_CONTROLLER_H_

#include <esp_http_server.h>
#include <vector>

// Controls relays that are active-low. */
class RelayController {
 public:
  RelayController(const std::vector<int>& mapping);
  bool switch_on(int idx, bool on);
  // Clicks with a default delay.
  bool click(int idx);
  // Clicks with given delay in milliseconds.
  bool click(int idx, int delay_millis);
 private:
  std::vector<int> mapping_;
  void initPin(int n);
};

#endif /* _WINSTON_RELAY_CONTROLLER_H_ */
