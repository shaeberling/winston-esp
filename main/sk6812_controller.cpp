#include "sk6812_controller.h"

#include "controller.h"
#include "esp_log.h"
#include "Adafruit_NeoPixel.h"

#include <string>
#include <vector>

static const char* TAG = "sk6812-ctrl";

SK6812Controller::SK6812Controller(const std::string& id,
                                   const gpio_num_t pin,
                                   Locking* locking) :
    id_(id),
    pin_(pin),
    locking_(locking),
    pixels_(NULL),
    initialized_(false),
    frame_(0),
    num_pixels_(10),
    update_delay_millis_(80) {
}

// override
bool SK6812Controller::init() {
  pixels_ = new Adafruit_NeoPixel(10, pin_, NEO_GRBW + NEO_KHZ800);
  pixels_->clear();
  pixels_->show();

  auto rt = xTaskCreatePinnedToCore(
     SK6812Controller::startUpdateLoop, "led-updater", 5000, this, 1, NULL, 0);
  if (rt != pdPASS) {
   ESP_LOGE(TAG, "Cannot create RGBW update task.");
   return false;
  }
  return true;
}

// override
void SK6812Controller::registerIO(std::vector<SensorConfig*>* sensors,
                                  std::vector<ActuatorConfig*>* actuators) {
  actuators->push_back(new ActuatorConfig {
    .name = "rgbw",
    .id = this->id_,
    .set_value = [this](const std::string& data) {
        return this->handleCommand(data);
    }
  });
}

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t w;
} Color;

// TODO: Define data format.
// private
bool SK6812Controller::handleCommand(const std::string& data) {
  ESP_LOGD(TAG, "RGBW Command: %s", data.c_str());

  return true;
}

// private
void SK6812Controller::onUpdateLeds() {
  Color col[10];
  col[0] = {0, 0, 0, 150};
  col[1] = {150, 0, 0, 0};
  col[2] = {150, 0, 0, 0};
  col[3] = {0, 0, 0, 150};
  col[4] = {0, 150, 0, 0};
  col[5] = {0, 150, 0, 0};
  col[6] = {0, 0, 0, 150};
  col[7] = {0, 0, 250, 0};
  col[8] = {0, 0, 250, 0};
  col[9] = {0, 0, 0, 150};

  // pixels_->clear();
  for (int i = 0; i < num_pixels_; ++i) {
    auto const c = col[(frame_ + i) % num_pixels_];
    pixels_->setPixelColor(i, pixels_->Color(c.r, c.g, c.b, c.w));
  }
  pixels_->show();
  frame_ = (frame_ + 1) % num_pixels_;
  vTaskDelay(update_delay_millis_ / portTICK_PERIOD_MS);
}

// private static
void SK6812Controller::startUpdateLoop(void* p) {
  while (true) {
    static_cast<SK6812Controller*>(p)->onUpdateLeds();
  }
}
