# Architecture

## Core flow

The main sketch `PumpController.ino` orchestrates all managers:

- `WiFiManager`: connects to Wi-Fi, can start captive portal fallback.
- `BlynkManager`: cloud sync for virtual pins and app-driven pump control.
- `ESPNowManager`: receives tank data and remote commands from peer devices.
- `PumpManager`: controls relay state and safety logic.
- `ButtonManager`: local physical button handling + manual override timer.
- `FlowSensorManager`: interrupt-based pulse counting.
- `TelegramManager`: sends alerts and handles approval/admin commands.

## Communication model

- **ESP-NOW**
  - `LEVEL` packets update tank level widgets (`V0`, `V1`).
  - `CMD` packets request pump `ON/OFF`, with command threshold logic.
  - `TELEGRAM` packets queue outgoing notification text.
- **Blynk**
  - App switch (`V2`) controls pump directly.
  - Pump state mirrored on `V3`.
- **Telegram**
  - Bot polling interval in main loop.
  - `/start`, `/approve`, `/unapprove`, `/listapproved` supported.
- **REST API** (when in AP mode)
  - `GET /api/levels` - Returns tank levels
  - `GET /api/status` - Returns pump state, levels, WiFi IP
  - `GET /api/pump?state=on|off` - Control pump

## Pump safety functionality

Safety logic in `PumpManager`:

- Auto-stop when runtime exceeds `MAX_PUMP_ON_DURATION_MS`.
- Cooldown block for restarts shorter than `PUMP_RESTART_COOLDOWN_MS`.
- Safety timeout event is pushed via Blynk notification.

## Auto-pump control

New automatic pump control based on tank levels:

- Pump turns ON when `tank1 >= TANK1_LOW_THRESHOLD` AND `tank2 < TANK2_HIGH_THRESHOLD`
- Pump turns OFF when `tank2 >= TANK2_HIGH_THRESHOLD` OR `tank1 < TANK1_LOW_THRESHOLD`
- Disabled when manual override is active
- Configure via `AUTO_PUMP_ENABLED`, `TANK1_LOW_THRESHOLD`, `TANK2_HIGH_THRESHOLD`

## OTA Updates

Built-in OTA (Over-The-Air) update support:

- Uses ArduinoOTA library (included in ESP32 core)
- Password: `OTA_PASSWORD` in Config.h
- Upload via Arduino IDE: Tools → Port → Network Ports → ESP32-xxxx

## Notes for public deployment

- Keep secrets only in local `Config.h`.
- `Config.example.h` is safe to commit and share.
- `.gitignore` excludes `Config.h` and common build artifacts.
