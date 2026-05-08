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

    pollTelegramUpdates();
  }

  if (!wifiManager.isConnected()) {
    wifiManager.handleDNS();
    webServerManager.handleClient();
  }
}

unsigned long lastUpdateId = 0;

void pollTelegramUpdates() {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  String url = "https://api.telegram.org/bot" + String(TELEGRAM_TOKEN) + "/getUpdates?offset=" + String(lastUpdateId + 1);

  if (https.begin(client, url)) {
    int httpCode = https.GET();
    if (httpCode == 200) {
      String payload = https.getString();

      const size_t capacity = 30 * JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(3) + 2048;
      DynamicJsonDocument doc(capacity);
      DeserializationError error = deserializeJson(doc, payload);

      if (!error && doc["ok"]) {
        JsonArray result = doc["result"].as<JsonArray>();

        for (JsonVariant update : result) {
          JsonObject obj = update.as<JsonObject>();
          lastUpdateId = obj["update_id"].as<unsigned long>();

          if (obj.containsKey("message")) {
            String chatId = obj["message"]["chat"]["id"].as<String>();
            String text = obj["message"]["text"].as<String>();
            Serial.println("Received: " + text + " from " + chatId);
            telegramManager.handleCommand(chatId, text);
          }
        }
      }
    }
    https.end();
  }
}
