#pragma once

#ifndef _WINSTON_SYSTEM_H_
#define _WINSTON_SYSTEM_H_

#include "controller.h"

#include <string>

class SystemController : public Controller {
 public:
  SystemController(const std::string& node_name);

  bool init() override;

  // From Controller interface, returns all "sensors".
  std::vector<SensorConfig*> getSensors() const override;

  // Get the amount of free heap memory, in bytes.
  int getFreeHeapBytes() const;

  // See vTaskGetRunTimeStats documentation.
  std::string getRunTimeStats() const;

  // Reboots the ESP.
  bool restart(int delay_millis);

  // Returns information about the system and H/W features.
  std::string getSystemInfo() const;

  // Get MAC address.
  std::string getMacAddress() const;

  // Get the node's name.
  std::string getNodeName() const;

 private:
  std::string node_name_;
};

#endif /* _WINSTON_SYSTEM_H_ */
