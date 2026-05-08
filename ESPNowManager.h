#ifndef ESPNOW_MANAGER_H
#define ESPNOW_MANAGER_H

#include <esp_now.h>
#include <WiFi.h>
#include "PumpManager.h"
#include "BlynkManager.h"

struct MessagePacket {
  char type[16];    // "LEVEL", "CMD", or "TELEGRAM"
  char payload[128];
};

class ESPNowManager {
public:
  static void begin();
  static String getPendingTelegramMessage();
  static void clearPendingTelegramMessage();

private:
  static void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len);
  static int onCount;
  static int offCount;
  static unsigned long onFirstCommandTime;
  static unsigned long offFirstCommandTime;
  static const int commandThreshold;
  static const unsigned long commandWindow;
  static String pendingTelegramMsg;
};

#endif
