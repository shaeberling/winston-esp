#pragma once

#ifndef _WINSTON_SYSTEM_H_
#define _WINSTON_SYSTEM_H_

#include <string>

class SystemController {
 public:
  SystemController();

  // Get the amount of free heap memory, in bytes.
  int getFreeHeapBytes();

  // See vTaskGetRunTimeStats documentation.
  std::string getRunTimeStats();
};

#endif /* _WINSTON_SYSTEM_H_ */
