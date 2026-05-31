#include "Config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

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
  telegramManager.sendMessage("ESP32 Pump Controller started. IP: " + wifiManager.getIPAddress(), TELEGRAM_CHAT_ID);
  PumpManager::begin();
  FlowSensorManager::begin();
  ButtonManager::begin();
  ESPNowManager::begin();
}

void loop() {
  unsigned long currentMillis = millis();
  blynkManager.run();
  FlowSensorManager::update();
  ButtonManager::update();
  PumpManager::update();

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
