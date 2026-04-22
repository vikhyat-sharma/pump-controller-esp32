#include "WebServerManager.h"
#include "WiFiManager.h"

WebServerManager webServerManager;

void WebServerManager::startServer() {
  server.on("/api/levels", HTTP_GET, [this]() {

    float percentage1 = 100;
    float percentage2 = 100;
    String json = "{\"tank1\":" + String(percentage1, 1) + ",\"tank2\":" + String(percentage2, 1) + "}";
    server.send(200, "application/json", json);
  });

  server.onNotFound([this]() {
    server.send(404, "text/plain", "Not Found");
  });

  server.begin();
  Serial.println("Web Server started.");
}

void WebServerManager::handleClient() {
  server.handleClient();
}
