#include "time_controller.h"

#include "esp_log.h"
#include "esp_sntp.h"

#include <stdio.h>
#include <string>
#include <sys/time.h>
#include <time.h>


namespace {
static const char *TAG = "win-time-controller";

void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "Time synchronization event");
}


}  // namespace

TimeController::TimeController(const std::string& timezone)
    :timezone_(timezone) {
}

// public
void TimeController::syncWithNtp() {
  // TODO: Make timezone configurable at some point, or try to get it from
  // public IP automatically (though might not be perfect all the time).
  // Valid values:
  // https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
  setenv("TZ", timezone_.c_str(), 1);
  tzset();

  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.org");
  sntp_set_time_sync_notification_cb(time_sync_notification_cb);
  sntp_init();
}

std::string TimeController::getTimeAndDate() {
  time_t now;
  struct tm t;
  time(&now);
  localtime_r(&now, &t);

  char buffer[50];
  std::sprintf(buffer, "%04d/%02d/%02d %02d:%02d",
               t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
               t.tm_hour, t.tm_min);
  return std::string(buffer);
}