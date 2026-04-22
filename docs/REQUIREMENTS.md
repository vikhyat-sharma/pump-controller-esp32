# Requirements

## Hardware

- ESP32 development board (tested with ESP32-WROOM class boards)
- Relay module suitable for your pump control circuit
- Push button for manual override
- Flow sensor connected to `FLOW_SENSOR_PIN` (optional but supported)
- Remote level-sensing node(s) sending ESP-NOW packets

## Software

- Arduino IDE 2.x (or PlatformIO with equivalent ESP32 support)
- ESP32 board package for Arduino
- A Blynk IoT account/project
- Telegram bot token and admin chat ID (optional, if Telegram integration is used)

## Arduino libraries

Install these via Library Manager:
- `Blynk`
- `ArduinoJson`

ESP32 core already includes:
- `WiFi`
- `HTTPClient`
- `WiFiClientSecure`
- `esp_now`
- `Preferences`
- `WebServer`
- `DNSServer`

## Configuration

1. Copy `Config.example.h` to `Config.h`.
2. Set at minimum:
   - `WIFI_SSID`, `WIFI_PASS`
   - `BLYNK_TEMPLATE_ID`, `BLYNK_TEMPLATE_NAME`, `BLYNK_DEVICE_NAME`, `BLYNK_AUTH_TOKEN`
3. Optional Telegram setup:
   - `TELEGRAM_TOKEN`, `TELEGRAM_CHAT_ID`
4. Tune runtime safety values if needed:
   - `MAX_PUMP_ON_DURATION_MS`
   - `PUMP_RESTART_COOLDOWN_MS`
