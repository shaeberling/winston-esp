#pragma once

#ifndef _WINSTON_TIME_CONTROLLER_H_
#define _WINSTON_TIME_CONTROLLER_H_

#include <string>

class TimeController {
 public:
  TimeController(const std::string& timezone);
  void syncWithNtp();
  std::string getTimeAndDate();

 private:
  std::string timezone_;
};

#endif /* _WINSTON_TIME_CONTROLLER_H_ */
