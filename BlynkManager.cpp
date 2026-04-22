#include "BlynkManager.h"
#include "Config.h"
#include <BlynkSimpleEsp32.h>
#include "PumpManager.h"

void BlynkManager::begin(const char* auth, const char* ssid, const char* pass) {
  Blynk.begin(auth, ssid, pass);
}

void BlynkManager::run() {
  Blynk.run();
}

void BlynkManager::sendVirtual(int pin, int value) {
  Blynk.virtualWrite(pin, value);
}

void BlynkManager::sendNotification(const String& message) {
  Blynk.logEvent("notification", message);  // assumes you have a notification event created in Blynk cloud dashboard
}

BLYNK_WRITE(V2) {
  int buttonState = param.asInt();
  if (buttonState == 1)
    PumpManager::turnOn();
  else
    PumpManager::turnOff();
}