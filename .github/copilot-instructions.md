# ESP32-S3 Lawnmower Project Instructions

## 1. Quick Context & Purpose
- **Goal:** Autonomous/Manual lawnmower firmware driving a Hoverboard base + Brushless Blade Motor.
- **Hardware:** ESP32-S3 DevKitC-1, MPU6050 (I2C), Hoverboard Mainboard (UART), 120A ESC (Blade).
- **Single Source of Truth:** `src/ManualLawnmover.cpp` contains all hardware specs, pinout definitions, battery config, and skill documentation. **Consult this file before changing hardware-related constants.**

## 2. Architecture (Dual-Core FreeRTOS)
The system is designed to isolate critical drive logic from network/monitoring tasks.

| Task Name | Core | Prio | Function |
|-----------|------|------|----------|
| `controlLogicTask` | 1 | 4 | **CRITICAL**: Main drive loop (PID, Speed/Steer ramp), sends UART to Hoverboard (30ms). |
| `WiFiTask` | 1 | 5 | HTTP Server, WebSocket handling, DNS. High priority to maintain connectivity. |
| `loop()` | 1* | - | UART RX (Hoverboard), Safety Checks (`handleSystemStatus`), Serial Input. (*Default Arduino Task) |
| `MPUTask` | 0 | 0 | Reads MPU6050 I2C data (50Hz). Critical for capsize detection. |
| `StatusTask` | 0 | 2 | Slow monitoring (ADC voltages, Temps), WebSocket Broadcasts (2s). |
| `HeartbeatTask` | 0 | 0 | ESP-NOW keepalive packets. |
| `ESPNowTask` | 0 | 5 | ESP-NOW callback registration & management. |

**Key Data Flows:**
- **Drive:** `controlLogicTask` -> UART TX -> Hoverboard.
- **Feedback:** Hoverboard -> UART RX -> `loop()` -> Global Structs.
- **Sensors:** MPU6050 -> `MPUTask` -> Global `ypr` array.

## 3. Critical Patterns & Conventions
- **Hardware Abstraction:** Do not hardcode pins. Use constants from `ManualLawnmover.cpp` (e.g., `BLADE_ESC_PIN`, `HOVER_RX_PIN`).
- **Safety Systems:**
  - **Capsize:** `monitorSkill8Angles()` checks pitch/roll. If >60°, triggers `skill8SafetyActive` (stop).
  - **Blade Interlock:** Blade only spins if `currentBladeHeight == BLADE_HEIGHT_DOWN` (simulated or real).
  - **Voltage/Temp:** `handleSystemStatus()` monitors limits and triggers E-Stop.
- **Web UI:**
  - Located in `src/webpage.cpp` (PROGMEM).
  - Uses `AsyncWebServer` and WebSockets (`/ws`).
  - **Caution:** Large JSON payloads in `onWsEvent` can crash the stack. Use `DynamicJsonDocument` carefully.
  - **Sync Rule:** After modifying `src/webpage.cpp`, **ALWAYS** run `python tools/extract_preview.py` to update `preview.html`. This allows the user to view UI changes without flashing.
- **Debugging:**
  - Use `debugPrintln()` / `debugPrintf()` instead of `Serial.print()`. This buffers output to the WebLog for remote debugging.
- **MPU Initialization:**
  - `setup()` contains a robust retry loop (2 attempts) for MPU6050. Do not remove this.

## 4. Development Workflow
- **Build:** `pio run`
- **Upload:** `pio run -t upload` (USB) or OTA.
- **Monitor:** `pio device monitor` (115200 baud).
- **Web UI Sync:** `python tools/extract_preview.py` (Run after editing `src/webpage.cpp`).
- **OTA:** Hostname `Lawnmower-ESP32`, Password `1234`.
- **Testing:**
  - Use **Skill 999** (Web-Serial Bridge) to debug Hoverboard UART without USB.
  - Always test drive changes with **Blade Motor Disconnected** or **Blade Removed**.

## 5. Known Issues & Stress Points
- **Mutex Contention:** `espNowMutex` and `i2cMutex` share resources between cores. Use short timeouts.
- **WiFi/ESP-NOW:** Shared radio channel (Fixed to Ch 6). `onDataReceive` runs in interrupt context - keep it short!
- **Memory:** `huge_app.csv` partition scheme is required (3MB App / 1MB SPIFFS).
