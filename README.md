# Pump Controller (ESP32)

A simple and beginner-friendly **ESP32-based smart pump controller** for automated water tank management.

---

## Quick Start

1. **Copy Config.example.h to Config.h**
   ```bash
   cp Config.example.h Config.h
   ```

2. **Edit Config.h** - Fill in your:
   - WiFi name & password
   - Blynk auth token (from blynk.io)
   - Telegram bot token & your chat ID

3. **Upload** - Open `PumpController.ino` in Arduino IDE and upload to ESP32

---

## How It Works

The controller combines several ways to control your pump:

| Control Method | How to Use |
|----------------|------------|
| **Manual Button** | Press the physical button connected to GPIO 25 |
| **Blynk App** | Use switch on V2 to turn pump on/off |
| **Telegram** | Send `/on` or `/off` commands |
| **Auto Mode** | Pump starts/stops based on tank levels |

---

## File Structure (Simple!)

```
PumpController/
├── PumpController.ino    # All main logic - THIS IS THE CORE FILE
├── PumpController.h      # Header with pin definitions
├── Config.example.h      # Edit this with your credentials
├── Config.h              # Your personal settings (not committed)
├── docs/                 # Technical documentation
└── LegacyManagers/       # Original separate files (optional)
```

**That's it!** Most users only need to edit the 2 config files.

---

## Pin Connections

| GPIO | Purpose |
|------|---------|
| 27   | Pump Relay (ON = LOW) |
| 25   | Manual Override Button |
| 26   | Flow Sensor Pulse Input |
| 2    | Built-in LED (Status) |

---

## Blynk Dashboard Setup

Create a free project at [blynk.io](https://blynk.io) and add these widgets:

| Widget | Virtual Pin | Purpose |
|--------|-------------|---------|
| Switch | V2 | Control pump |
| LED/Value | V3 | Show pump status |
| LED/Value | V0 | Tank 1 level |
| LED/Value | V1 | Tank 2 level |
| LED/Value | V4 | Flow rate |

---

## Telegram Commands

Send these to your bot:

- `/start` - See welcome message
- `/on` - Turn pump ON (admin only)
- `/off` - Turn pump OFF (admin only)
- `/status` - View current status

---

## Safety Features

- **Auto-stop**: Pump turns off after 30 minutes (configurable)
- **Cooldown**: 30-second delay between pump restarts
- **Manual Override**: Physical button always works

---

## Troubleshooting

**WiFi won't connect?**
- Check SSID and password in Config.h
- Device will start a hotspot if WiFi fails

**Pump not starting?**
- Check relay wiring (most are active-LOW)
- Verify manual override isn't stuck on

**Blynk not working?**
- Verify auth token is correct
- Check WiFi is connected

---

## License

MIT License - See LICENSE file