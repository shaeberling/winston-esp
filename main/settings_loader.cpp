#include "settings_loader.h"

#include <string>
#include <vector>

#include "esp_log.h"

#include "device_settings.pb.h"
#include "pb_decode.h"

static const char* TAG = "win-settings";

namespace {

// Called from nanopb to parse components of the message.
static bool pb_component_callback(pb_istream_t* stream,
                                  const pb_field_t* field,
                                  void** arg) {
  auto* settings = static_cast<Settings*>(*arg);
  ComponentProto component;
  if (!pb_decode(stream, ComponentProto_fields, &component)) {
    ESP_LOGE(TAG, "Failed to decode component");
    return false;
  }
  settings->components.push_back(component);
  return true;
}

// Called from nanopb to parse the message.
static bool pb_read_callback(pb_istream_t* stream, uint8_t* buf, size_t count) {
  char* data_start = (char*) stream->state;
  memcpy(buf, data_start, count);
  stream->state = data_start + count;
  return true;
}

// Create a nanopb input stream that rad the node_config.bin file.
pb_istream_t pb_istream_from_file() {
  extern const unsigned char node_config_start[] asm("_binary_node_config_bin_start");
  extern const unsigned char node_config_end[]   asm("_binary_node_config_bin_end");
  auto length = node_config_end - node_config_start;
  pb_istream_t stream = {&pb_read_callback,
                         (void*)node_config_start,
                         static_cast<size_t>(length),
                         NULL};
  return stream;
}

}  // namespace


SettingsLoader::SettingsLoader() { }

bool SettingsLoader::loadSettings(Settings* settings) {
  ESP_LOGI(TAG, "Reading embedded node config ...");
  settings->device_settings.component.arg = settings;
  settings->device_settings.component.funcs.decode = &pb_component_callback;
  pb_istream_t sizestream = pb_istream_from_file();
  bool success = pb_decode(&sizestream, DeviceSettingsProto_fields, &settings->device_settings);
  if (!success) {
    ESP_LOGE(TAG, "Error decoding embedded binary node config.");
    return false;
  }
  return true;
}