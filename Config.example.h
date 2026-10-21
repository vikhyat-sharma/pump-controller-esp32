// ============================================================================
// CONFIGURATION FILE - EDIT THIS!
// ============================================================================
// Copy this file to Config.h and fill in your values.
// DO NOT commit Config.h to version control!
// ============================================================================

#ifndef CONFIG_H
#define CONFIG_H

// -------------------------------------------------------------------------
// SECTION 1: WIFI CREDENTIALS
// -------------------------------------------------------------------------
// Your home WiFi network name and password
#define WIFI_SSID        "YourWiFiName"
#define WIFI_PASS        "YourWiFiPassword"

// -------------------------------------------------------------------------
// SECTION 2: BLYNK APP (https://blynk.io)
// -------------------------------------------------------------------------
// Get these from your Blynk project settings
#define BLYNK_AUTH_TOKEN "YourBlynkAuthToken"

// Optional: Template ID and Name (for new Blynk apps)
#define BLYNK_TEMPLATE_ID   "YourTemplateID"
#define BLYNK_TEMPLATE_NAME "PumpController"

// -------------------------------------------------------------------------
// SECTION 3: TELEGRAM BOT
// -------------------------------------------------------------------------
// Create a bot via @BotFather on Telegram
#define TELEGRAM_TOKEN  "1234567890:ABCdefGHIjklMNOpqrsTUVwxyz"

// Your Telegram chat ID (find via @userinfobot)
#define TELEGRAM_CHAT_ID "123456789"

// -------------------------------------------------------------------------
// SECTION 4: HARDWARE PINS
// -------------------------------------------------------------------------
// GPIO pin numbers - change if you wired differently
#define PUMP_RELAY_PIN       27   // Relay module (ON = LOW)
#define MANUAL_BUTTON_PIN    25   // Push button
#define FLOW_SENSOR_PIN      26   // Flow meter pulses

// -------------------------------------------------------------------------
// SECTION 5: PUMP SETTINGS
// -------------------------------------------------------------------------
// Safety: stop pump automatically after this many milliseconds
// 1800000 = 30 minutes (90% of max capacity for typical 500GPD pump)
#define MAX_PUMP_ON_DURATION_MS  1800000UL

// Cooldown: wait this long before allowing pump restart (prevents rapid cycling)
// 30000 = 30 seconds
#define PUMP_RESTART_COOLDOWN_MS 30000UL

// -------------------------------------------------------------------------
// SECTION 6: BLYNK VIRTUAL PINS
// -------------------------------------------------------------------------
// You can see these in your Blynk dashboard
#define V0  0   // Tank 1 % (from ESP-NOW sensor)
#define V1  1   // Tank 2 % (from ESP-NOW sensor)
#define V2  2   // Pump ON/OFF switch (your control)
#define V3  3   // Pump status (shows ON/OFF)
#define V4  4   // Flow pulse count
#define V5  5   // Runtime counter / alerts

// -------------------------------------------------------------------------
// SECTION 7: AUTO-PUMP THRESHOLDS
// -------------------------------------------------------------------------
// When to automatically start/stop pump based on tank levels
#define TANK1_LOW_THRESHOLD   20   // Source tank must be at least 20% full
#define TANK2_HIGH_THRESHOLD  90   // Stop when destination reaches 90% full

// Set to 1 to enable auto-pump, 0 to disable
#define AUTO_PUMP_ENABLED     1

// -------------------------------------------------------------------------
// SECTION 8: OTA UPDATES
// -------------------------------------------------------------------------
// Password for wireless firmware updates (via Arduino IDE)
#define OTA_PASSWORD  "esp32update"

#endif