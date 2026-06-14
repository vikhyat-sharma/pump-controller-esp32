#include "ESPNowManager.h"
#include <Arduino.h>

int ESPNowManager::onCount = 0;
int ESPNowManager::offCount = 0;
unsigned long ESPNowManager::onFirstCommandTime = 0;
unsigned long ESPNowManager::offFirstCommandTime = 0;
const int ESPNowManager::commandThreshold = 3;
const unsigned long ESPNowManager::commandWindow = 30000UL;
String ESPNowManager::pendingTelegramMsg = "";

void ESPNowManager::begin() {
  WiFi.mode(WIFI_STA);
  Serial.print("Controller MAC: "); Serial.println(WiFi.macAddress());
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  esp_now_register_recv_cb(onDataRecv);
}

String ESPNowManager::getPendingTelegramMessage() {
  return pendingTelegramMsg;
}

void ESPNowManager::clearPendingTelegramMessage() {
  pendingTelegramMsg = "";
}

void ESPNowManager::onDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  if (len != sizeof(MessagePacket)) {
    Serial.println("Invalid message size.");
    return;
  }

  MessagePacket msg;
  memcpy(&msg, incomingData, sizeof(MessagePacket));
  msg.type[sizeof(msg.type) - 1] = '\0';
  msg.payload[sizeof(msg.payload) - 1] = '\0';

  Serial.print("Received Type: "); Serial.println(msg.type);
  Serial.print("Payload: "); Serial.println(msg.payload);

  if (strcmp(msg.type, "LEVEL") == 0) {
    int tank1Level = 0;
    int tank2Level = 0;
    sscanf(msg.payload, "%d,%d", &tank1Level, &tank2Level);

    Serial.print("Tank 1: "); Serial.print(tank1Level);
    Serial.print(" | Tank 2: "); Serial.println(tank2Level);

    PumpManager::updateTankLevels(tank1Level, tank2Level);
  } else if (strcmp(msg.type, "TELEGRAM") == 0) {
    pendingTelegramMsg = String(msg.payload);
    pendingTelegramMsg.trim();
    Serial.print("Queued TELEGRAM Message: ");
    Serial.println(pendingTelegramMsg);
  } else if (strcmp(msg.type, "CMD") == 0) {
    if (PumpManager::manualOverride) {
      Serial.println("Manual override active. Ignoring remote command.");
      return;
    }

    String command = String(msg.payload);
    command.trim();
    unsigned long currentMillis = millis();

    if (command == "ON") {
      if (currentMillis - onFirstCommandTime > commandWindow) {
        onCount = 0;
        onFirstCommandTime = currentMillis;
      }
      onCount++;
      offCount = 0;
      Serial.print("ON count: "); Serial.println(onCount);
      if (onCount >= commandThreshold) {
        PumpManager::turnOn();
        onCount = 0;
      }
    } else if (command == "OFF") {
      if (currentMillis - offFirstCommandTime > commandWindow) {
        offCount = 0;
        offFirstCommandTime = currentMillis;
      }
      offCount++;
      onCount = 0;
      Serial.print("OFF count: "); Serial.println(offCount);
      if (offCount >= commandThreshold) {
        PumpManager::turnOff();
        offCount = 0;
      }
    } else {
      onCount = 0;
      offCount = 0;
    }
  }
}
