/* HTTP server for the Winston REST API. */
#include "server.h"

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

#include "esp_http_server.h"
#include "request_handler.h"

static const char *TAG = "winston-server";

namespace {

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err) {
  ESP_LOGI(TAG, "404 ERROR");
  httpd_resp_send_err(req, HTTPD_404_NOT_FOUND,
                      "I'm sorry, Dave, I'm afraid I can't do that.");
  return ESP_OK;
}

void split(const std::string& str,
           const char delim,
           std::vector<std::string>* items) {
  std::stringstream ss(str);
  std::string token;
  while (std::getline(ss, token, delim)) {
      items->push_back(token);
  }
}

}  // namespace

Server::Server(int port, RequestHandler* request_handler)
    :port_(port),
     server_(NULL),
     request_handler_(request_handler) {
  // Configure ESP httpd's request handlers.
  io_handler_.uri = "/io/*";
  io_handler_.method   = HTTP_GET;
  io_handler_.handler  = Server::io_get_handler;
  io_handler_.user_ctx = this;
};

// public
bool Server::start() {
  ESP_LOGI(TAG, "Starting server on port %d", port_);
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  // Use the URI wildcard matching function in order to
  // allow the same handler to respond to multiple different
  // target URIs which match the wildcard scheme.
  config.uri_match_fn = httpd_uri_match_wildcard;

  // Start the httpd server.
  if (httpd_start(&server_, &config) == ESP_OK) {
    // Set up URI handlers.
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server_, &io_handler_);
    httpd_register_err_handler(server_, HTTPD_404_NOT_FOUND, &http_404_error_handler);
    return true;
  }
  return false;
}

// public
void Server::stop() {
    httpd_stop(server_);
}

// private static
esp_err_t Server::io_get_handler(httpd_req_t *req) {
  ((Server*)req->user_ctx)->handle_io(req);
  return ESP_OK;
}

// private
esp_err_t Server::handle_io(httpd_req_t *req) {
  std::string uri(req->uri);
  auto resp_str = this->request_handler_->handle(uri);
  httpd_resp_send(req, resp_str.c_str(), resp_str.length()); 
  return ESP_OK;
}
