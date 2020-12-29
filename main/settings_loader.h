#pragma once

#ifndef _WINSTON_TIME_SETTINGS_LOADER_H_
#define _WINSTON_TIME_SETTINGS_LOADER_H_

#include <string>

#include "device_settings.pb.h"
#include "pb_decode.h"


class SettingsLoader {
 public:
  explicit SettingsLoader();
  bool loadSettings(DeviceSettingsProto* settings);
};

#endif /* _WINSTON_TIME_SETTINGS_LOADER_H_ */
