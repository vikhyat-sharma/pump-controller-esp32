#include "FlowSensorManager.h"

volatile int FlowSensorManager::pulseCount = 0;

void IRAM_ATTR flowPulseISR() {
  FlowSensorManager::pulseCount++;
}

void FlowSensorManager::begin() {
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), flowPulseISR, FALLING);
}

void FlowSensorManager::update() {
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    if (pulseCount != 0) {
      BlynkManager::sendVirtual(VFLOW_PULSES, pulseCount);
    }
    pulseCount = 0;
  }
}