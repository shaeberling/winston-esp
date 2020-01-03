#pragma once

#ifndef _WINSTON_SERVER_H_
#define _WINSTON_SERVER_H_

#include <esp_http_server.h>

class Server {
 public:
  Server(int port) : port_(port), server_(NULL) {};
  bool start();
  void stop();
 private:
  int port_;
  httpd_handle_t server_;
};

#endif /* _WINSTON_SERVER_H_ */
