/* HTTP server for the Winston REST API. */
#include "server.h"

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

// Handles /io requests.
static esp_err_t io_get_handler(httpd_req_t *req) {
  const char* resp_str = "Hello, Winston ESP here.";
  httpd_resp_send(req, resp_str, strlen(resp_str));
  return ESP_OK;
}

// Configure request handlers.
static const httpd_uri_t io_handler = {
  .uri      = "/io/*",
  .method   = HTTP_GET,
  .handler  = io_get_handler,
  .user_ctx = NULL
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
    httpd_register_uri_handler(server_, &io_handler);
    httpd_register_err_handler(server_, HTTPD_404_NOT_FOUND, &http_404_error_handler);
    return true;
  }
  return false;
}

// public
void Server::stop() {
    httpd_stop(server_);
}
