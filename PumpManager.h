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
  static int tank1Level;  // Current tank 1 level percentage
  static int tank2Level;  // Current tank 2 level percentage

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
    pinMode(PUMP_RELAY_PIN, OUTPUT);
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

  // Auto-pump control based on tank levels
  static void checkAutoPump() {
#if AUTO_PUMP_ENABLED == 1
    if (manualOverride) {
      return;
    }

    // Turn ON pump if tank1 is above low threshold AND tank2 is below high threshold
    if (!pumpState && tank1Level >= TANK1_LOW_THRESHOLD && tank2Level < TANK2_HIGH_THRESHOLD) {
      Serial.println("Auto-pump: Turning ON (tank1 OK, tank2 needs filling)");
      turnOn();
    }
    // Turn OFF pump if tank2 is full or tank1 is too low
    else if (pumpState && (tank2Level >= TANK2_HIGH_THRESHOLD || tank1Level < TANK1_LOW_THRESHOLD)) {
      Serial.println("Auto-pump: Turning OFF (tank2 full or tank1 low)");
      turnOff();
    }
#endif
  }

  // Update tank levels from ESP-NOW
  static void updateTankLevels(int t1, int t2) {
    tank1Level = constrain(t1, 0, 100);
    tank2Level = constrain(t2, 0, 100);
    BlynkManager::sendVirtual(VTANK1_LEVEL, tank1Level);
    BlynkManager::sendVirtual(VTANK2_LEVEL, tank2Level);
  }
};

#endif
