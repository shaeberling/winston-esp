#pragma once

#ifndef _WINSTON_RELAY_CONTROLLER_H_
#define _WINSTON_RELAY_CONTROLLER_H_

#include <esp_http_server.h>
#include <vector>

class RelayController {
 public:
  RelayController(const std::vector<int>& mapping);
  void switch_on(int idx, bool on);
 private:
  std::vector<int> mapping_;
  void initPin(int n);
};

#endif /* _WINSTON_RELAY_CONTROLLER_H_ */
