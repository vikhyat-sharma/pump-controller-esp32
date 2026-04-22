# Pump Controller (ESP32)

An ESP32-based smart pump controller for water tank automation with:
- local manual override button,
- remote command/input via ESP-NOW,
- cloud monitoring/control through Blynk,
- Telegram notifications and command handling.

## What is new

- Added a **pump safety timeout** (`MAX_PUMP_ON_DURATION_MS`) to auto-stop long-running pump sessions.
- Added a **restart cooldown** (`PUMP_RESTART_COOLDOWN_MS`) to prevent rapid relay/pump cycling.
- Removed hardcoded credentials from tracked docs and code examples.

## Quick Start

1. Create your local config:
   - Copy `Config.example.h` to `Config.h`.
   - Fill your Wi-Fi, Blynk, and Telegram credentials in `Config.h`.
2. Open `PumpController.ino` in Arduino IDE.
3. Install required libraries listed in [`docs/REQUIREMENTS.md`](docs/REQUIREMENTS.md).
4. Select your ESP32 board and upload.

## Blynk virtual pins

| Pin | Purpose |
|---|---|
| `V2` | Pump control switch |
| `V3` | Pump status indicator |
| `V4` | Flow pulse count |
| `V5` | Safety timeout runtime (seconds) |
| `V0` | Tank 1 level (from ESP-NOW) |
| `V1` | Tank 2 level (from ESP-NOW) |

## Project details

Detailed requirements and architecture are split into separate docs:
- [`docs/REQUIREMENTS.md`](docs/REQUIREMENTS.md)
- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md)

## Security note

Do not commit `Config.h` to GitHub. This repo includes `.gitignore` rules to prevent leaking secrets.

## License

This project is licensed under the [MIT License](LICENSE).
