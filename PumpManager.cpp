#include "PumpManager.h"
bool PumpManager::pumpState = false;
bool PumpManager::manualOverride = false;
unsigned long PumpManager::manualOverrideStartTime = 0;
unsigned long PumpManager::pumpOnStartTime = 0;
unsigned long PumpManager::lastPumpOffTime = 0;
