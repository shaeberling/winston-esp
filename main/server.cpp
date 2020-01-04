/* HTTP server for the Winston REST API. */
#include "server.h"

#include <string>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"

#include <esp_http_server.h>

static const char *TAG = "winston-server";

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err) {
  ESP_LOGI(TAG, "404 ERROR");
  httpd_resp_send_err(req, HTTPD_404_NOT_FOUND,
                      "I'm sorry, Dave, I'm afraid I can't do that.");
  return ESP_OK;
}

Server::Server(int port,
               ReedController* reed_controller,
               RelayController* relay_controller)
    :port_(port),
     server_(NULL),
     reed_controller_(reed_controller),
     relay_controller_(relay_controller) {
  // Configure request handlers.
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
  std::string requestStr((const char*)(req->uri + sizeof("/io") - 1));
  if (requestStr.rfind("/reed/", 0) == 0) {
    std::string req_data((const char*)(req->uri + sizeof("/io/reed/") - 1));
    bool closed = is_reed_closed(req_data);
    httpd_resp_send(req, closed ? "1" : "0", 1);
  } else if (requestStr.rfind("/relay/", 0) == 0) {
    std::string req_data((const char*)(req->uri + sizeof("/io/relay/") - 1));
    bool success = switch_relay_on(req_data);
    auto resp = success ? "OK" : "FAIL";
    httpd_resp_send(req, resp, strlen(resp));
  } else {
    const char* resp_str = "Hello, Winston ESP here.";
    httpd_resp_send(req, resp_str, strlen(resp_str));    
  }
  return ESP_OK;
}


// private
bool Server::is_reed_closed(const std::string& req) {
  ESP_LOGI(TAG, "Geet reed status for '%s'", req.c_str());
  char* pEnd = NULL;
  int reed_idx = strtod(req.c_str(), &pEnd);
  if (*pEnd) {
    ESP_LOGW(TAG, "Invalid reed index.");
    return false;
  }
  return reed_controller_->is_closed(reed_idx);
}

// private
bool Server::switch_relay_on(const std::string& req) {
  ESP_LOGI(TAG, "Switch relay '%s'", req.c_str());

  // Format at this point should be  <relay>/<on>
  std::size_t found = req.rfind("/");
  if (found == std::string::npos) {
    ESP_LOGW(TAG, "Invalid relay switch request: Missing switch component.");
    return false;
  }

  auto relay_idx_str = req.substr(0, found);
  ESP_LOGI(TAG, "Switch relay idx '%s'", relay_idx_str.c_str());
  char* pEnd = NULL;
  int relay_idx = strtod(relay_idx_str.c_str(), &pEnd);
  if (*pEnd) {
    ESP_LOGW(TAG, "Invalid relay index.");
    return false;
  }

  auto relay_switch_cmd = req.substr(found + 1);
  ESP_LOGI(TAG, "Switch command is '%s'", relay_switch_cmd.c_str());
  if (relay_switch_cmd == "0") {
    return relay_controller_->switch_on(relay_idx, false);
  } else if (relay_switch_cmd == "1") {
    return relay_controller_->switch_on(relay_idx, true);
  } else if (relay_switch_cmd == "2") {
    return relay_controller_->click(relay_idx);
  } else {
    ESP_LOGW(TAG, "Invalid relay switch value.");
    return false;
  }
}
