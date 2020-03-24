#pragma once

#ifndef _WINSTON_TEMP_CONTROLLER_H_
#define _WINSTON_TEMP_CONTROLLER_H_

// Get temperature from the built-in ESP32 temp sensor.
class TempController {
 public:
  TempController();
  float get_celsius(int idx);
};

#endif /* _WINSTON_TEMP_CONTROLLER_H_ */
