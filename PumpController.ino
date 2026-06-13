#include "Config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

#include "BlynkManager.h"
#include "PumpManager.h"
#include "FlowSensorManager.h"
#include "ButtonManager.h"
#include "ESPNowManager.h"
#include "WiFiManager.h"
#include "TelegramManager.h"

// Blynk credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;

unsigned long previousNotificationMillis = 0;
unsigned long notificationIntervalMillis = 2 * 60 * 1000;  // 2 minutes

void setup() {
  Serial.begin(115200);
  blynkManager.begin(auth, ssid, pass);

  wifiManager.connectWiFi();
  
  // Setup OTA updates
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    Serial.println("OTA Update Start: " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA Update Complete");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
  });
  ArduinoOTA.begin();
  
  telegramManager.sendMessage("ESP32 Pump Controller started. IP: " + wifiManager.getIPAddress(), TELEGRAM_CHAT_ID);
  PumpManager::begin();
  FlowSensorManager::begin();
  ButtonManager::begin();
  ESPNowManager::begin();
}

void loop() {
  unsigned long currentMillis = millis();
  blynkManager.run();
  ArduinoOTA.handle();  // OTA update handler
  FlowSensorManager::update();
  ButtonManager::update();
  PumpManager::update();
  PumpManager::checkAutoPump();  // Auto-pump based on tank levels

  String telegramMsg = ESPNowManager::getPendingTelegramMessage();
  if (telegramMsg.length() > 0) {
    if (!wifiManager.isConnected()) {
      wifiManager.connectWiFi();
    }

    telegramManager.sendToApproved(telegramMsg);
    ESPNowManager::clearPendingTelegramMessage();
  }

  if (currentMillis - previousNotificationMillis >= notificationIntervalMillis) {
    previousNotificationMillis = currentMillis;
    if (!wifiManager.isConnected()) {
      wifiManager.connectWiFi();
    }

    telegramManager.pollUpdates();
  }

  if (!wifiManager.isConnected()) {
    wifiManager.handleDNS();
    webServerManager.handleClient();
  }
}

unsigned long lastUpdateId = 0;

// Telegram polling moved into TelegramManager::pollUpdates()
