/* HTTP server for the Winston REST API. */
#include "mongoose_server.h"

#include <sstream>
#include <string>
#include <vector>

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"

#include "mongoose.h"
#include "request_handler.h"


static const char *TAG = "winston-mongoose-server";

MongooseServer::MongooseServer(int port, RequestHandler* request_handler)
    :port_(port),
     request_handler_(request_handler),
     mgr_(NULL),
     stop_(false) {
};

// public
bool MongooseServer::start() {
  ESP_LOGI(TAG, "Starting server on port %d", port_);

  this->mgr_ = new mg_mgr();
  // Initialize event manager object
  mg_mgr_init(mgr_, NULL);
  struct mg_bind_opts opts;
  memset(&opts, 0, sizeof(opts));
  opts.user_data = this;

  auto* conn = mg_bind_opt(mgr_,
                           std::to_string(port_).c_str(),
                           MongooseServer::mg_ev_handler,
                           opts);
  if (conn == NULL) {
    ESP_LOGE(TAG, "Error setting up listener!");
    return false;
  }
  mg_set_protocol_http_websocket(conn);

  // Asynchronously start the serving loop.
  auto rt = xTaskCreatePinnedToCore(MongooseServer::mg_loop_task, "mg-server", 5000, this, 1, NULL, 0);
  return rt == pdPASS;
}

// public
void MongooseServer::stop() {
  ESP_LOGI(TAG, "Stopping Mongoose server ...");
  stop_ = true;
  const auto delay_millis = 100;
  while (stop_) {
    vTaskDelay(delay_millis / portTICK_PERIOD_MS);
  }
  ESP_LOGI(TAG, "Mongoose server stopped.");
}

// private static
void MongooseServer::mg_loop_task(void* p) {
  static_cast<MongooseServer*>(p)->start_loop();
}

// private
void MongooseServer::start_loop() {
  while (!stop_) {
    mg_mgr_poll(mgr_, 1000);
  }
  mg_mgr_free(mgr_);
  stop_ = false;
  vTaskDelete(NULL);
}

// private static
void MongooseServer::mg_ev_handler(struct mg_connection *conn, int ev, void *p) {
  if (ev == MG_EV_HTTP_REQUEST) {
    static_cast<MongooseServer*>(conn->user_data)->handle_request(
        conn, static_cast<struct http_message*>(p));
  }
}

// private
void MongooseServer::handle_request(struct mg_connection* conn,
                                    struct http_message* hm) {
  std::string uri = std::string(hm->uri.p, hm->uri.len);
  auto resp_str = this->request_handler_->handle(uri);

  mg_send_head(conn, 200, resp_str.length(), "Content-Type: text/plain");
  mg_printf(conn, "%.*s", (int)resp_str.length(), resp_str.c_str());
}
