#include "WebServerManager.h"
#include "WiFiManager.h"
#include "PumpManager.h"

WebServerManager webServerManager;

void WebServerManager::onLevelsRequest() {
  server.on("/api/levels", HTTP_GET, [this]() {
    String json = "{\"tank1\":" + String(PumpManager::tank1Level) + ",\"tank2\":" + String(PumpManager::tank2Level) + "}";
    server.send(200, "application/json", json);
  });

  server.on("/api/status", HTTP_GET, [this]() {
    String json = "{\"pump\":" + String(PumpManager::pumpState ? "true" : "false") +
                  ",\"tank1\":" + String(PumpManager::tank1Level) +
                  ",\"tank2\":" + String(PumpManager::tank2Level) +
                  ",\"manualOverride\":" + String(PumpManager::manualOverride ? "true" : "false") +
                  ",\"wifiIP\":\"" + WiFi.localIP().toString() + "\"}";
    server.send(200, "application/json", json);
  });

  server.on("/api/pump", HTTP_GET, [this]() {
    if (server.hasArg("state")) {
      String state = server.arg("state");
      if (state == "on") {
        PumpManager::turnOn();
      } else if (state == "off") {
        PumpManager::turnOff();
      }
    }
    server.send(200, "application/json", "{\"pump\":" + String(PumpManager::pumpState ? "true" : "false") + "}");
  });

  server.onNotFound([this]() {
    server.send(404, "text/plain", "Not Found");
  });
}

void WebServerManager::startServer() {
  onLevelsRequest();
  server.begin();
  Serial.println("Web Server started.");
}

void WebServerManager::handleClient() {
  server.handleClient();
}
