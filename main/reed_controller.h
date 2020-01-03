#pragma once

#ifndef _WINSTON_REED_CONTROLLER_H_
#define _WINSTON_REED_CONTROLLER_H_

#include <esp_http_server.h>
#include <vector>

class ReedController {
 public:
  ReedController(const std::vector<int>& mapping);
  bool is_closed(int idx);
 private:
  std::vector<int> mapping_;
  void initPin(int n);
};

#endif /* _WINSTON_REED_CONTROLLER_H_ */
