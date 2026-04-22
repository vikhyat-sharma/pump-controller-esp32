#ifndef BLYNK_MANAGER_H
#define BLYNK_MANAGER_H

#include <Arduino.h>

class BlynkManager {
public:
  static void begin(const char* auth, const char* ssid, const char* pass);
  static void run();
  static void sendVirtual(int pin, int value);
  static void sendNotification(const String& message);
};

extern BlynkManager blynkManager;
#endif
