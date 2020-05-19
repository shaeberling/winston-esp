#pragma once

#ifndef _WINSTON_SERVER_H_
#define _WINSTON_SERVER_H_

#include <string>
#include <esp_err.h>
#include <esp_http_server.h>

#include "request_handler.h"

class Server {
 public:
  Server(int port, RequestHandler* request_handler);
  bool start();
  void stop();

 private:
  int port_;
  httpd_handle_t server_;
  RequestHandler* request_handler_;

  httpd_uri_t io_handler_;

  static esp_err_t io_get_handler(httpd_req_t *req);
  esp_err_t handle_io(httpd_req_t *req);
};

#endif /* _WINSTON_SERVER_H_ */
