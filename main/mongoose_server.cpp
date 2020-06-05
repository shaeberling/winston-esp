/* HTTP server for the Winston REST API. */
#include "mongoose_server.h"

#include <sstream>
#include <string>
#include <vector>

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>

#include "mongoose.h"
#include "request_handler.h"

namespace {

#define LOG_ACTIVE_CONNECTIONS false

int countActiveConnections(struct mg_mgr* mgr) {
  int count = 0;
  for (struct mg_connection *nc = mgr->active_connections;
       nc != NULL; nc = nc->next) {
    count++;
  }
  return count;
}

}  // namespace


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
                           MongooseServer::mgEvHandler,
                           opts);
  if (conn == NULL) {
    ESP_LOGE(TAG, "Error setting up listener!");
    return false;
  }
  mg_set_protocol_http_websocket(conn);

  // Asynchronously start the serving loop.
  auto rt = xTaskCreatePinnedToCore(MongooseServer::mgLoopTask, "mg-server", 5000, this, 1, NULL, 0);
  return rt == pdPASS;
}

// public
void MongooseServer::stop() {
  stop_ = true;
  const auto delay_millis = 100;
  while (stop_) {
    vTaskDelay(delay_millis / portTICK_PERIOD_MS);
  }
}

// private static
void MongooseServer::mgLoopTask(void* p) {
  static_cast<MongooseServer*>(p)->startLoop();
}

// private
void MongooseServer::startLoop() {
  while (!stop_) {
    mg_mgr_poll(mgr_, 1000);
    if (LOG_ACTIVE_CONNECTIONS) {
      ESP_LOGI(TAG, "Active connections: %d.", countActiveConnections(mgr_));
    }
  }
  mg_mgr_free(mgr_);
  stop_ = false;
  vTaskDelete(NULL);
}

// private static
void MongooseServer::mgEvHandler(struct mg_connection *conn, int ev, void *p) {
  if (ev == MG_EV_HTTP_REQUEST) {
    static_cast<MongooseServer*>(conn->user_data)->handleRequest(
        conn, static_cast<struct http_message*>(p));
  }
}

// private
void MongooseServer::handleRequest(struct mg_connection* conn,
                                   struct http_message* hm) {
  std::string uri = std::string(hm->uri.p, hm->uri.len);
  auto resp_str = this->request_handler_->handle(uri);
  int code = resp_str == "Bad request." ? 400 : 200;

  mg_send_head(conn, code, resp_str.length(), "Content-Type: text/plain"
                                              "Connection: close");
  mg_printf(conn, "%.*s", (int)resp_str.length(), resp_str.c_str());
}
