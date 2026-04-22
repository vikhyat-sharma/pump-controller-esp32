#ifndef ESPNOW_MANAGER_H
#define ESPNOW_MANAGER_H

#include <esp_now.h>
#include <WiFi.h>
#include "PumpManager.h"
#include "BlynkManager.h"
#include "TelegramManager.h"

struct MessagePacket {
  char type[16];    // "LEVEL" or "CMD"
  char payload[128];
};
int onCount = 0;
int offCount = 0;
unsigned long onFirstCommandTime = 0;
unsigned long offFirstCommandTime = 0;
const int commandThreshold = 3;
const unsigned long commandWindow = 30000;  // 30 seconds
String pendingTelegramMsg = "";

class ESPNowManager {
public:
  static void begin() {
    WiFi.mode(WIFI_STA);
    Serial.print("Controller MAC: "); Serial.println(WiFi.macAddress());
    if (esp_now_init() != ESP_OK) {
      Serial.println("ESP-NOW init failed");
      return;
    }
    esp_now_register_recv_cb(onDataRecv);
  }

  static void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
    if (len != sizeof(MessagePacket)) {
      Serial.println("Invalid message size.");
      return;
    }

    MessagePacket msg;
    memcpy(&msg, incomingData, sizeof(MessagePacket));

    Serial.print("Received Type: "); Serial.println(msg.type);
    Serial.print("Payload: "); Serial.println(msg.payload);

    if (String(msg.type) == "LEVEL") {
      int tank1Level, tank2Level;
      sscanf(msg.payload, "%d,%d", &tank1Level, &tank2Level);

      Serial.print("Tank 1: "); Serial.print(tank1Level);
      Serial.print(" | Tank 2: "); Serial.println(tank2Level);

      BlynkManager::sendVirtual(VTANK1_LEVEL, tank1Level);
      BlynkManager::sendVirtual(VTANK2_LEVEL, tank2Level);
    }

    else if (String(msg.type) == "TELEGRAM") {
      pendingTelegramMsg = String(msg.payload); 
      Serial.print("Queued TELEGRAM Message: ");
      Serial.println(pendingTelegramMsg);
    }

    else if (String(msg.type) == "CMD") {
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
          onFirstCommandTime = currentMillis;  // reset starting point
        }
        onCount++;
        offCount = 0;
        Serial.print("ON count: "); Serial.println(onCount);
        if (onCount >= commandThreshold) {
          PumpManager::turnOn();
          onCount = 0;  // reset after acting
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
};

#endif
