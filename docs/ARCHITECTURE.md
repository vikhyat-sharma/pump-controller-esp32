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

## Pump safety functionality

New safety logic in `PumpManager`:

- Auto-stop when runtime exceeds `MAX_PUMP_ON_DURATION_MS`.
- Cooldown block for restarts shorter than `PUMP_RESTART_COOLDOWN_MS`.
- Safety timeout event is pushed via Blynk notification.

## Notes for public deployment

- Keep secrets only in local `Config.h`.
- `Config.example.h` is safe to commit and share.
- `.gitignore` excludes `Config.h` and common build artifacts.
