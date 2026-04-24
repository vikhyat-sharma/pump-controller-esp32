# Pump Controller (ESP32)

A reliable and extensible **ESP32-based smart pump controller** designed for automated water tank management. This system integrates local control, wireless communication, and cloud connectivity to deliver a robust and safe pump operation experience.

---

## Overview

The Pump Controller enables efficient monitoring and control of water pumps using multiple interfaces:

* **Manual Control**: Physical button for local override
* **ESP-NOW Communication**: Low-latency, peer-to-peer data exchange between devices
* **Blynk Integration**: Cloud-based dashboard for remote monitoring and control
* **Telegram Integration**: Real-time alerts and command-based interaction

This multi-channel design ensures both reliability and flexibility in real-world deployments.

---

## What's New

Recent updates focus on improving system safety and maintainability:

* **Pump Safety Timeout**
  Automatically stops the pump after a configurable maximum runtime (`MAX_PUMP_ON_DURATION_MS`) to prevent damage or dry running.

* **Restart Cooldown Protection**
  Introduces a delay (`PUMP_RESTART_COOLDOWN_MS`) between successive pump activations to avoid rapid relay switching and hardware stress.

* **Improved Security Practices**
  Removed hardcoded credentials from source files and documentation. Configuration is now handled securely via a local configuration file.

---

## Getting Started

Follow these steps to set up and run the project:

1. **Create Configuration File**

   * Copy `Config.example.h` → `Config.h`
   * Add your credentials:

     * Wi-Fi
     * Blynk
     * Telegram

2. **Open Project**

   * Launch `PumpController.ino` in the Arduino IDE

3. **Install Dependencies**

   * Refer to `docs/REQUIREMENTS.md` for required libraries

4. **Upload to Device**

   * Select your ESP32 board
   * Compile and upload the firmware

---

## Blynk Virtual Pins

| Virtual Pin | Description                      |
| ----------- | -------------------------------- |
| `V2`        | Pump control switch              |
| `V3`        | Pump status indicator            |
| `V4`        | Flow pulse counter               |
| `V5`        | Safety timeout runtime (seconds) |
| `V0`        | Tank 1 level (via ESP-NOW)       |
| `V1`        | Tank 2 level (via ESP-NOW)       |

---

## Project Documentation

Detailed technical information is available in the following documents:

* `docs/REQUIREMENTS.md` – Dependencies and setup requirements
* `docs/ARCHITECTURE.md` – System design and component interactions

---

## Security Considerations

* **Do not commit `Config.h`** to version control
* Sensitive credentials (Wi-Fi, API keys, tokens) must remain local
* The repository includes `.gitignore` rules to prevent accidental exposure

---

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.
