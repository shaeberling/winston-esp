#pragma once

#ifndef _WINSTON_TIME_SETTINGS_LOADER_H_
#define _WINSTON_TIME_SETTINGS_LOADER_H_

#include <string>
#include <vector>

#include "device_settings.pb.h"
#include "pb_decode.h"

struct Settings {
  DeviceSettingsProto device_settings;
  std::vector<ComponentProto> components; 
};

class SettingsLoader {
 public:
  explicit SettingsLoader();
  bool loadSettings(Settings* settings);
};

#endif /* _WINSTON_TIME_SETTINGS_LOADER_H_ */
