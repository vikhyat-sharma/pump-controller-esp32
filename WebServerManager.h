#ifndef WEBSERVERMANAGER_H
#define WEBSERVERMANAGER_H

#include <WebServer.h>
#include "Config.h"

class WebServerManager {
public:
  void startServer();
  void handleClient();
  // Register any additional handlers before calling startServer()
  void onLevelsRequest();

private:
  WebServer server = WebServer(80);
};

extern WebServerManager webServerManager;

#endif
