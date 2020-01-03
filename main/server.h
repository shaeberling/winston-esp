#pragma once

#ifndef _WINSTON_SERVER_H_
#define _WINSTON_SERVER_H_

#include <string>
#include <esp_err.h>
#include <esp_http_server.h>

#include "reed_controller.h"

class Server {
 public:
  Server(int port, ReedController* reed_controller);
  bool start();
  void stop();
 private:
  int port_;
  httpd_handle_t server_;
  ReedController* reed_controller_;
  httpd_uri_t io_handler_;

  static esp_err_t io_get_handler(httpd_req_t *req);
  esp_err_t handle_io(httpd_req_t *req);
  bool get_reed_status(const std::string& req);
};

#endif /* _WINSTON_SERVER_H_ */
