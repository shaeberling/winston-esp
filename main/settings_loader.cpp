#include "settings_loader.h"

#include <string>

#include "esp_log.h"

#include "device_settings.pb.h"
#include "pb_decode.h"

static const char* TAG = "win-settings";

namespace {

// Called from nanopb to parse components of the message.
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

bool SettingsLoader::loadSettings(DeviceSettingsProto* settings) {
  ESP_LOGI(TAG, "Reading embedded node config ...");
  pb_istream_t sizestream = pb_istream_from_file();
  bool success = pb_decode(&sizestream, DeviceSettingsProto_fields, settings);
  if (!success) {
    ESP_LOGE(TAG, "Error decoding embedded binary node config.");
    return false;
  }
  return true;
}