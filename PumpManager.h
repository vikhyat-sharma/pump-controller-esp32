#ifndef PUMP_MANAGER_H
#define PUMP_MANAGER_H

#include <Arduino.h>
#include "Config.h"
#include "BlynkManager.h"

class PumpManager {
public:
  static bool pumpState;
  static bool manualOverride;
  static unsigned long manualOverrideStartTime;
  static unsigned long pumpOnStartTime;
  static unsigned long lastPumpOffTime;

  static void begin() {
    pinMode(PUMP_RELAY_PIN, OUTPUT);
    digitalWrite(PUMP_RELAY_PIN, HIGH); // Relay OFF (Active LOW)
    BlynkManager::sendVirtual(VPUMP_STATUS, pumpState);
  }

  static void turnOn() {
    if (PUMP_RESTART_COOLDOWN_MS > 0 && (millis() - lastPumpOffTime < PUMP_RESTART_COOLDOWN_MS)) {
      Serial.println("Pump restart blocked by cooldown.");
      return;
    }

    pinMode(PUMP_RELAY_PIN, OUTPUT);
    digitalWrite(PUMP_RELAY_PIN, LOW);
    if (!pumpState) {
      pumpState = true;
      pumpOnStartTime = millis();
      BlynkManager::sendVirtual(VPUMP_STATUS, 1);
      Serial.println("Pump turned ON");
    }
  }

  static void turnOff() {
    pinMode(PUMP_RELAY_PIN, INPUT);  // High impedance / OFF
    digitalWrite(PUMP_RELAY_PIN, HIGH);
    if (pumpState) {
      pumpState = false;
      lastPumpOffTime = millis();
      BlynkManager::sendVirtual(VPUMP_STATUS, 0);
      Serial.println("Pump turned OFF");
    }
  }

  static void toggle() {
    if (pumpState)
      turnOff();
    else
      turnOn();

    Serial.print("Pump State: ");
    Serial.println(pumpState ? "ON" : "OFF");
  }

  static void update() {
    if (!pumpState) {
      return;
    }

    const unsigned long elapsed = millis() - pumpOnStartTime;
    if (MAX_PUMP_ON_DURATION_MS > 0 && elapsed >= MAX_PUMP_ON_DURATION_MS) {
      turnOff();
      Serial.println("Safety timeout reached: Pump auto-stopped.");
      BlynkManager::sendNotification("Pump auto-stopped due to safety timeout.");
      BlynkManager::sendVirtual(V5, static_cast<int>(elapsed / 1000));
    }
  }
};

#endif
