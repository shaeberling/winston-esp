#pragma once

#ifndef _WINSTON_REED_CONTROLLER_H_
#define _WINSTON_REED_CONTROLLER_H_

#include <esp_http_server.h>

class ReedController {
 public:
  bool status(int idx);
};

#endif /* _WINSTON_REED_CONTROLLER_H_ */
