#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>
#include "Config.h"
#include "PumpManager.h"

class ButtonManager {
public:
  static unsigned long lastDebounceTime;

  static void begin() {
    pinMode(MANUAL_OVERRIDE_BUTTON_PIN, INPUT_PULLUP);
  }

  static void update() {
    int buttonState = digitalRead(MANUAL_OVERRIDE_BUTTON_PIN);
    if (buttonState == LOW) {
      if (millis() - lastDebounceTime > DEBOUNCE_DELAY) {
        PumpManager::toggle();

        // Enable manual override on button press
        PumpManager::manualOverride = true;
        PumpManager::manualOverrideStartTime = millis();

        Serial.println("Manual override enabled by button.");
        lastDebounceTime = millis();
      }
      while (digitalRead(MANUAL_OVERRIDE_BUTTON_PIN) == LOW);  // wait for release
    }

    // Auto-reset override after 10 minutes (600,000 ms)
    if (PumpManager::manualOverride && (millis() - PumpManager::manualOverrideStartTime >= 600000)) {
      PumpManager::manualOverride = false;
      Serial.println("Manual override expired after 10 minutes.");
    }
  }
};

#endif
