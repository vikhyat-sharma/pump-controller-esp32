#include <Arduino.h>
#include "WiFiManager.h"
#include <DNSServer.h>
#include "WebServerManager.h"

DNSServer dnsServer;
const byte DNS_PORT = 53;

WiFiManager wifiManager;

void WiFiManager::connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");

  unsigned long startAttemptTime = millis();
  const unsigned long wifiTimeout = 15000;

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connect failed — starting Captive Portal.");
    startAccessPoint();
  }
}

String WiFiManager::getIPAddress() {
  return WiFi.localIP().toString();
}

bool WiFiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::startAccessPoint() {
  Serial.println("Starting Access Point...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_AP);
  delay(100);
  bool result = WiFi.softAP("ESP32-Setup", "12345678", 6);
  if (result) {
    Serial.println("Access Point started successfully.");
  } else {
    Serial.println("Failed to start Access Point.");
  }

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Start DNS redirect server
  dnsServer.start(DNS_PORT, "*", myIP);

  // Serve captive portal pages
  webServerManager.server.on("/", HTTP_GET, []() {
    webServerManager.server.send(200, "text/html", "<h2>Welcome to ESP32 Captive Portal</h2>");
  });

  webServerManager.server.on("/generate_204", HTTP_GET, []() {
    webServerManager.server.send(200, "text/html", "<h2>Captive Portal Detected</h2>");
  });

  webServerManager.server.on("/hotspot-detect.html", HTTP_GET, []() {
    webServerManager.server.send(200, "text/html", "<h2>Captive Portal Detected</h2>");
  });

  webServerManager.server.on("/success.html", HTTP_GET, []() {
    webServerManager.server.send(200, "text/html", "<h2>Captive Portal Detected</h2>");
  });

  webServerManager.server.onNotFound([]() {
    webServerManager.server.send(200, "text/html", "<h2>ESP32 Captive Portal</h2>");
  });

  webServerManager.server.begin();
  Serial.println("Captive portal web server started.");
}

void WiFiManager::handleDNS() {
  dnsServer.processNextRequest();
}
