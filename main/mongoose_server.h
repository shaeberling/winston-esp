#pragma once

#ifndef _WINSTON_MONGOOSE_SERVER_H_
#define _WINSTON_MONGOOSE_SERVER_H_

#include <string>

#include "request_handler.h"

class MongooseServer {
 public:
  MongooseServer(int port, RequestHandler* request_handler);
  bool start();
  void stop();

 private:
  int port_;
  RequestHandler* request_handler_;
  struct mg_mgr* mgr_;
  bool stop_;

  static void mgLoopTask(void* p);
  void startLoop();
  static void mgEvHandler(struct mg_connection* conn, int ev, void *p);
  void handleRequest(struct mg_connection* conn, struct http_message* hm);
};

#endif /* _WINSTON_MONGOOSE_SERVER_H_ */
