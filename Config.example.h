#ifndef CONFIG_H
#define CONFIG_H

// Copy values from your own Blynk project.
#define BLYNK_TEMPLATE_ID "YOUR_BLYNK_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_BLYNK_TEMPLATE_NAME"
#define BLYNK_DEVICE_NAME "YOUR_BLYNK_DEVICE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

// Telegram bot values from BotFather and your chat.
#define TELEGRAM_TOKEN "YOUR_TELEGRAM_BOT_TOKEN"
#define TELEGRAM_CHAT_ID "YOUR_TELEGRAM_ADMIN_CHAT_ID"

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

#define PUMP_RELAY_PIN 27
#define MANUAL_OVERRIDE_BUTTON_PIN 25
#define FLOW_SENSOR_PIN 26

#define DEBOUNCE_DELAY 50
// Safety timeout: force stop after long runtime.
#define MAX_PUMP_ON_DURATION_MS 1800000UL
// Optional cooldown to prevent rapid re-starts.
#define PUMP_RESTART_COOLDOWN_MS 30000UL

#define V0 0   // for tank 1
#define V1 1   // for tank 2
#define V2 2   // for pump control (Blynk switch)
#define V3 3   // for pump status
#define V4 4   // for flow pulses
#define V5 5   // runtime status / safety notifications

#define VPUMP_STATUS V3
#define VPUMP_CONTROL V2
#define VFLOW_PULSES V4

#define VTANK1_LEVEL V0
#define VTANK2_LEVEL V1

// Tank level thresholds for auto-pump control (percentage 0-100)
#define TANK1_LOW_THRESHOLD 20
#define TANK2_HIGH_THRESHOLD 90
#define AUTO_PUMP_ENABLED 1  // Set to 1 to enable auto-pump based on levels

// OTA Update
#define OTA_PASSWORD "esp32update"

#endif
