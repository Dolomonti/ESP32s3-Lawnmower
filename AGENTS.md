# ESP32 Lawnmower Control System - Agent Documentation

## Project Overview

This is an **ESP32-S3 based robotic lawnmower control system** built with PlatformIO. The project controls a custom-built autonomous/remote-controlled lawnmower featuring:

- **Drive System**: Hoverboard hub motors (2x 350W) controlled via UART
- **Blade System**: Brushless motor (SURPASS-HOBBY C3548-V2, 900KV) with 120A ESC
- **IMU**: MPU6050 for orientation sensing and capsize protection
- **Communication**: WiFi Web Interface + ESP-NOW for remote control
- **Power**: Separate battery systems for drive (10S Li-Ion, 36V) and blade (4S Li-Ion, ~14.4V)

> **Language Note**: The project uses a mix of English and German (comments, variable names, documentation). German terms like "Klinge" (blade), "Steuerung" (control), and "Auslösewinkel" (trigger angle) are commonly used.

---

## Technology Stack

| Component | Technology |
|-----------|------------|
| **Platform** | PlatformIO with Arduino framework |
| **Board** | ESP32-S3-DevKitC-1 |
| **Framework** | Arduino-ESP32 |
| **Partition Scheme** | huge_app.csv (3MB No OTA / 1MB SPIFFS) |
| **Web Server** | ESPAsyncWebServer-esphome |
| **WebSocket** | AsyncWebSocket for real-time communication |
| **IMU** | MPU6050 with DMP (Digital Motion Processor) |
| **JSON** | ArduinoJson library |
| **Servo Control** | ESP32Servo for blade ESC |

---

## Build Commands

```bash
# Build the project
pio run -e esp32s3

# Upload firmware
pio run --target upload -e esp32s3

# Monitor serial output
pio device monitor --baud 115200

# Clean build files
pio run --target clean -e esp32s3
```

---

## Project Structure

```
ESP32s3_Lawnmover_VS_Code_bladeDist1/
├── platformio.ini           # PlatformIO configuration
├── src/
│   ├── main.cpp            # Main application logic (~2600 lines)
│   ├── webpage.cpp         # Embedded web UI (HTML/CSS/JS as C++ strings)
│   ├── ManualLawnmover.cpp # Hardware documentation and technical manual
│   └── ManualRemote.cpp    # PS3 remote control documentation
├── lib/
│   ├── I2Cdev/             # I2C communication library
│   └── MPU6050/            # MPU6050 driver with DMP support
├── include/                # Header files (currently unused)
├── tools/
│   ├── embed_preview.py    # Embed preview.html into webpage.cpp
│   └── extract_preview.py  # Extract webpage.cpp to preview.html for editing
├── preview.html            # Live-editable web UI (extracted from webpage.cpp)
└── .vscode/                # VS Code workspace settings
```

---

## Key Source Files

### `src/main.cpp`
Main application with:
- **Setup()**: Initializes WiFi AP, ESP-NOW, EEPROM, MPU6050, Web Server
- **Loop()**: Main control loop with skill processing
- **Skill System**: 20+ different skills (commands) for mower control
- **Safety Systems**: Capsize detection, voltage monitoring, temperature limits
- **WebSocket Handler**: Real-time communication with web UI

### `src/webpage.cpp`
Contains the complete web interface as PROGMEM strings:
- `webpage_part1[]` and `webpage_part2[]`: Split to avoid C++ string limits
- HTML/CSS/JavaScript for the mobile-responsive control interface
- Tabs: Manual Control, Lawnmower Setup, AI Assistant, Data Recording, WiFi Setup, User Manual

### `src/ManualLawnmover.cpp`
Comprehensive technical documentation including:
- Hardware specifications (pinout, wiring)
- ESC programming guide (120A brushless)
- Command reference (Serial and JSON)
- Full skill API documentation
- Safety systems documentation

### `src/ManualRemote.cpp`
PS3 controller documentation:
- Button mapping for remote control
- Control modes (linear, staged, exponential)
- Safety features (deadman switch, emergency stop)

---

## GPIO Pinout

| GPIO | Type | Function | Details |
|------|------|----------|---------|
| 1 | Analog | Blade Battery Sense | Voltage divider, factor ~6.07 |
| 2 | Output | Onboard LED | Heartbeat/Status |
| 4 | Output | Blade ESC PWM | 1000-2000μs servo signal |
| 5 | Output | Blade Relay | Safety cutoff (active HIGH) |
| 10 | Input | Button 1 | Pullup, Test forward |
| 11 | Input | Button 2 | Pullup, Test reverse |
| 13 | I2C | MPU6050 SDA | Motion sensor data |
| 14 | I2C | MPU6050 SCL | Motion sensor clock |
| 15 | Input | E-Stop Button | Pullup, Physical emergency stop |
| 17 | UART TX | To Hoverboard | Green wire (RX) |
| 18 | UART RX | From Hoverboard | Blue wire (TX) |
| 19 | Output | Blade Unit Power | Controls blade power relay |
| 20 | Output | Drive Unit Power | Controls drive power relay |
| 35 | Output | Blade UP | Lift actuator up |
| 36 | Output | Blade DOWN | Lift actuator down |
| 46 | Input | MPU6050 INT | Data ready interrupt |

---

## Skill System API

The mower uses a "Skill" command system. Skills are triggered via:
- WebSocket JSON commands
- ESP-NOW from remote controllers
- Serial monitor input

| Skill | Name | Parameters | Description |
|-------|------|------------|-------------|
| 1 | System Reset | - | Stop all skills, reset gyro, clear E-Stop |
| 2 | 90° Turn | ±1 (direction) | Automated 90-degree turn |
| 3 | Hold the Line | - | Maintain heading using PD control |
| 4 | 180° Turn | ±1 (direction) | Automated 180-degree turn |
| 5 | Blade Lift | ±1 (up/down) | Move blade height actuator |
| 6 | Blade Power | ±1 (on/off), or RPM | Control blade motor |
| 7 | Set Limits | speed, steer | Configure max speed/steering |
| 8 | E-Stop/Capsize | 20=trigger, -20=reset | Emergency stop and capsize protection |
| 9 | ESP32 Reboot | - | Restart the controller |
| 10 | Set Blade Speeds | eco, work, max, reset | Configure PWM values |
| 11 | Cable Reset | RPM | Reset blade cable tension |
| 12 | Drive System Levels | Various voltage/temp limits | Safety thresholds |
| 13 | Blade System Levels | Various voltage/temp limits | Safety thresholds |
| 14 | Capsize Parameters | angle, timeout | Configure capsize detection |
| 15 | PD Gains | Kp, Kd | Steering stabilizer gains |
| 16 | Blade Battery Factor | factor×100 | Calibrate voltage reading |
| 17 | Drive Battery Factor | factor×1000 | Calibrate voltage reading |
| 18 | Drive Unit Power | on/off | Control drive power relay |
| 19 | Hoverboard Params | cmd, accel, brake | Configure drive behavior |
| 20 | Distance Mission | DistL, DistR, SpeedL, SpeedR, Accel, Brake | Precision movement |
| 255 | Heartbeat | - | Connection keep-alive |

---

## Safety Systems

### Capsize Protection (Skill 8)
- **Trigger**: Pitch or Roll > `capsizeAngle` (default 60°) for > `capsizeTimeout` (default 1500ms)
- **Action**: Immediate stop (0,0), blade relay cutoff, safety lock active
- **Recovery**: Must send "Stop/Reset" command (Skill 8, value -20)

### Voltage Protection
- **Drive System**:
  - < 31.0V: Critical shutdown
  - < 32.5V: Safety mode (half speed, blade off)
  - > 43.0V: High voltage E-stop (regen braking protection)
- **Blade System**:
  - < 12.0V: Shutdown
  - < 13.0V: Safety mode

### Temperature Protection
- Drive Board > 50°C: E-stop
- Drive Board < 1°C: E-stop (frost protection)

---

## Web Interface

The web UI is embedded in `webpage.cpp` and served via ESPAsyncWebServer:

### Tabs
1. **Manual Control**: Joystick, turn buttons, blade controls
2. **Lawnmower Setup**: Safety settings, speed limits, PD gains, voltage/temp thresholds
3. **AI Assistant**: Gemini API integration for help, Web Serial interface
4. **Data Recording**: Real-time graphing of sensor data (voltage, temperature, speed)
5. **WiFi Setup**: Configure home WiFi credentials
6. **User Manual**: Complete technical documentation

### Connection
- **AP Mode**: SSID "Lawnmower_Control", Password "123456789", IP 192.168.4.1
- **STA Mode**: Connects to home WiFi if configured
- **WebSocket**: Real-time bidirectional communication at `/ws`

---

## Development Workflow

### Modifying the Web UI

The web interface is stored as C++ strings in `src/webpage.cpp`. For easier editing:

```bash
# 1. Extract webpage.cpp to preview.html
python3 tools/extract_preview.py

# 2. Open preview.html in VS Code
# 3. Edit with live preview (right-click → "Show Preview")

# 4. When done, embed back to webpage.cpp
python3 tools/embed_preview.py

# 5. Build and upload
pio run -e esp32s3
pio run --target upload -e esp32s3
```

### Adding New Libraries

Add to `platformio.ini` under `[env:esp32s3]`:

```ini
lib_deps =
    madhephaestus/ESP32Servo @ ^1.2.1
    bblanchon/ArduinoJson @ ^6.21.0
    esphome/ESPAsyncWebServer-esphome @ ^3.2.2
    arduino-libraries/NTPClient @ ^3.2.1
    ; Add new library here
```

---

## Configuration Storage

Settings are persisted in EEPROM with CRC32 checksum:

- **EEPROM_SIZE**: 512 bytes
- **SSID_ADDR**: 0 (32 bytes for WiFi SSID)
- **PASSWORD_ADDR**: 32 (32 bytes for WiFi password)
- **SETTINGS_ADDR**: 100 (Settings struct with magic number 0xDEADBEEF)

Settings include: voltage thresholds, temperature limits, speed limits, PD gains, battery calibration factors.

---

## Communication Protocols

### UART to Hoverboard
- Baud rate: 115200
- Binary protocol with checksum (XOR)
- Command structure: start(0xABCD), cmdCode, steer, speed, maxSpeedL, maxSpeedR, accPct, brkPct, checksum

### ESP-NOW
- Fixed channel: 6 (for WiFi coexistence)
- Broadcast MAC addresses configured in `deviceList[]`
- Packet structure: start(0xABCD), steer, speed, commandCode, checksum

### WebSocket
- JSON-based protocol
- Command format: `{ "type": "command", "data": { "code": X, "value": Y } }`
- Joystick format: `{ "type": "joystick", "speed": N, "steer": N }`

---

## Testing

### Serial Monitor Commands
Type directly into the serial monitor (115200 baud):

```
# Drive commands (steer,speed)
0,200      # Forward at speed 200
100,200    # Forward with right turn
0,0        # Stop

# Skills
1          # System reset
2+         # 90° turn right
2-         # 90° turn left
3          # Hold the line
6          # Toggle blade
7,800,600  # Set max speed 800, max steer 600
9          # Reboot ESP32

reset      # Restart ESP32
```

### Web Serial Monitor
In the AI Assistant tab, use the Web Serial input with same commands as above.

---

## Known Limitations

1. **ArduinoSTL Incompatible**: The `ArduinoSTL` library must be ignored (configured in `platformio.ini`) as it conflicts with ESP32 core.

2. **Partition Size**: Uses "huge_app" partition scheme for maximum code space (3MB). OTA updates require different partition scheme.

3. **Embed Script Disabled**: The `embed_preview.py` script is currently disabled in `platformio.ini` (`extra_scripts` commented out) to prevent accidental overwrites of `webpage.cpp`.

4. **Web UI Split**: The webpage content is split into two parts (`webpage_part1` and `webpage_part2`) due to C++ string literal size limitations.

---

## External Resources

- **PlatformIO ESP32**: https://docs.platformio.org/en/latest/platforms/espressif32.html
- **ESPAsyncWebServer**: https://github.com/esphome/ESPAsyncWebServer
- **MPU6050 DMP**: Uses Jeff Rowberg's I2Cdev/MPU6050 libraries with DMP support
- **Hoverboard Firmware**: UART communication protocol from NiklasFauth's hoverboard firmware

---

## Author

Markus Montagner
