# ESP32-S3 Lawnmower Project Instructions (Senior Architect Edition)

## 1. Quick Context & Purpose

- **Goal:** Autonomous/Manual lawnmower firmware driving a Hoverboard base + Brushless Blade Motor.
- **Hardware:** ESP32-S3 DevKitC-1, MPU6050 (I2C), Hoverboard Mainboard (UART), 120A ESC (Blade).
- **Single Source of Truth:** `src/ManualLawnmover.cpp` contains all hardware specs, pinout definitions, battery config, and skill documentation. **Consult this file before changing hardware-related constants.**
  "Ich nutze PlatformIO. Achte darauf, Bibliotheken in der platformio.ini zu verwalten."

## 2. Architecture (Dual-Core FreeRTOS)

The system is designed to isolate critical drive logic from network/monitoring tasks.

| Task Name          | Core | Prio | Function                                                                                |
| ------------------ | ---- | ---- | --------------------------------------------------------------------------------------- |
| `controlLogicTask` | 1    | 4    | **CRITICAL**: Main drive loop (PID, Speed/Steer ramp), sends UART to Hoverboard (30ms). |
| `WiFiTask`         | 1    | 5    | HTTP Server, WebSocket handling, DNS. High priority to maintain connectivity.           |
| `loop()`           | 1\*  | -    | UART RX (Hoverboard), Safety Checks (`handleSystemStatus`), Serial Input.               |
| `MPUTask`          | 0    | 0    | Reads MPU6050 I2C data (50Hz). Critical for capsize detection.                          |
| `StatusTask`       | 0    | 2    | Slow monitoring (ADC voltages, Temps), WebSocket Broadcasts (2s).                       |
| `HeartbeatTask`    | 0    | 0    | ESP-NOW keepalive packets.                                                              |
| `ESPNowTask`       | 0    | 5    | ESP-NOW callback registration & management.                                             |

**Key Data Flows:**

- **Drive:** `controlLogicTask` -> UART TX -> Hoverboard.
- **Feedback:** Hoverboard -> UART RX -> `loop()` -> Global Structs.
- **Sensors:** MPU6050 -> `MPUTask` -> Global `ypr` array.

## 3. Senior Developer Operational Guards (NEW)

**These rules ensure system stability and prevent hardware destruction:**

- **I2C Mutex Mandatory:** Every I2C access (MPU6050) MUST be wrapped in `i2cMutex` using `xSemaphoreTake`. Concurrent access from `MPUTask` and `StatusTask` on Core 0 will cause crashes.
- **Non-Blocking Logic:** NEVER use `delay()` in Core 1 tasks. Use `vTaskDelay()` or non-blocking `millis()` timers to ensure the 30ms UART heartbeat to the Hoverboard is never interrupted.
- **Safety Interlocks:** Any modification to Blade logic MUST check `skill8SafetyActive` and `currentBladeHeight`. The blade must NEVER spin if the mower is capsized (>60°) or the blade is in the UP position.
- **Atomic Updates:** When updating PID gains or calibration factors (Skills 15, 16, 17), ensure changes are atomic to prevent the controller from using partially updated values.

## 4. Critical Patterns & Conventions

- **Hardware Abstraction:** Do not hardcode pins. Use constants from `ManualLawnmover.cpp` (e.g., `BLADE_ESC_PIN`, `HOVER_RX_PIN`).
- **Safety Systems:**
  - **Capsize:** `monitorSkill8Angles()` triggers `skill8SafetyActive` if Pitch/Roll > `capsizeAngle`.
  - **Voltage/Temp:** `handleSystemStatus()` triggers E-Stop if battery <31.0V (Drive) or <12.0V (Blade).
- **Web UI:**
  - Code resides in `src/webpage.cpp` (PROGMEM).
  - **Sync Rule:** After modifying `src/webpage.cpp`, **ALWAYS** run `python tools/extract_preview.py` to update `preview.html`.
- **Debugging:** Use `debugPrintln()` / `debugPrintf()` (mirrors to Web-Log via Skill 998).
- **MPU Initialization:** `setup()` retry loop (2 attempts) is mandatory for DMP stability.

## 5. Development Workflow

- **Build/Upload:** `pio run` / `pio run -t upload` (USB or OTA).
- **OTA:** Hostname `Lawnmower-ESP32`, Password `1234`.
- **Testing:** Use **Skill 999** (Web-Serial Bridge) for direct Hoverboard/Skill interaction.
- **Pre-Flight:** Always test drive code with Blade Motor disconnected.

## 6. Known Issues & Stress Points

- **Mutex Contention:** `espNowMutex` and `i2cMutex` shared between cores. Use short timeouts (max 10ms).
- **WiFi/ESP-NOW Conflict:** Shared radio (Channel 6). `onDataReceive` is an ISR context—keep it extremely short.
- **Memory:** `huge_app.csv` (3MB App / 1MB SPIFFS) is required.
