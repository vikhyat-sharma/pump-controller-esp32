#ifndef FLOW_SENSOR_MANAGER_H
#define FLOW_SENSOR_MANAGER_H

#include <Arduino.h>
#include "Config.h"
#include "BlynkManager.h"

class FlowSensorManager {
public:
  static volatile int pulseCount;

  static void begin();
  static void update();
};

#endif

