#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>
#include "Config.h"
#include "PumpManager.h"

class ButtonManager {
public:
  static unsigned long lastDebounceTime;
  static int lastButtonState;

  static void begin() {
    pinMode(MANUAL_OVERRIDE_BUTTON_PIN, INPUT_PULLUP);
    lastButtonState = digitalRead(MANUAL_OVERRIDE_BUTTON_PIN);
    lastDebounceTime = millis();
  }

  static void update() {
    int buttonState = digitalRead(MANUAL_OVERRIDE_BUTTON_PIN);
    if (buttonState != lastButtonState) {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime > DEBOUNCE_DELAY) && buttonState == LOW && lastButtonState == HIGH) {
      PumpManager::toggle();
      PumpManager::manualOverride = true;
      PumpManager::manualOverrideStartTime = millis();
      Serial.println("Manual override enabled by button.");
    }

    lastButtonState = buttonState;

    if (PumpManager::manualOverride && (millis() - PumpManager::manualOverrideStartTime >= 600000)) {
      PumpManager::manualOverride = false;
      Serial.println("Manual override expired after 10 minutes.");
    }
  }
};

#endif
