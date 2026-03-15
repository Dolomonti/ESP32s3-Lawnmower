# LAWNMOWER MASTER TECHNICAL MANUAL (v5.0)

> **Partition Scheme:** "Huge APP (3MB No OTA/1MB SPIFFS)" or "Minimal SPIFFS (1.9MB APP with OTA)"
> **OTA Password:** `1234`

---

## SECTION 1: HARDWARE & ELECTRONICS SPECIFICATIONS

### 1.1 Core Computing
- **Microcontroller:** ESP32-S3 Dev Kit
- **IMU Sensor:** MPU6050 (I2C Address 0x68)
  - SDA → GPIO 13
  - SCL → GPIO 14
  - INT → GPIO 46 (Essential for DMP stability)

### 1.2 Drive System (Hoverboard)
| Spec | Value |
|------|-------|
| Motors | 2x Brushless Hub Motors (350W+ each) |
| Controller | Hoverboard Mainboard with UART firmware |
| Batterietyp | Samsung 35E (Li-Ion) |
| Konfiguration | 10S2P |
| Nennspannung | 36.0 V |
| Max. Ladespannung | 42.0 V |
| Nennkapazität | 17.5 Ah (2 × 3500mAh) |
| BMS Modell | Daly Li-Ion 10S 36V |
| Dauerentladestrom | 20 A (Zellen 2×8A) |
| Max. Entladestrom | 26 A (2 × 13A Peak) |
| Entladeschlussspannung | 26.5 V (BMS 27V) |
| Communication | UART Serial (115200 Baud) |
| ESP32 TX (GPIO 17) | → Hoverboard GREEN Wire (RX) |
| ESP32 RX (GPIO 18) | → Hoverboard BLUE Wire (TX) |

### 1.3 Blade System (The Cutter)
| Spec | Value |
|------|-------|
| Motor | SURPASS-HOBBY C3548-V2 (Outrunner) |
| Spec | 900KV, 14 Poles, 1000W Max |
| Voltage Range | 7-18V (2S-4S LiPo) |
| Max Current | 60A |
| ESC | 120A Brushless ESC (T/XT60 Plug) |
| Batterietyp | Samsung 35E (Li-Ion) |
| Konfiguration | 4S6P |
| Nennspannung | 14.4 V |
| BMS Modell | Daly Li-Ion 4S 12V 40A |
| ESC Signal | GPIO 4 (1000us = Stop, 2000us = Full) |
| Safety Relay | GPIO 5 (Active HIGH = ON) |

### 1.4 Pinout Reference

> **Quelle:** `src/main.cpp` (Zeilen 126-143, 1682)

| GPIO | Type | Function | Details | Variable Name |
|------|------|----------|---------|---------------|
| 1 | Analog | Blade Battery Sense | Voltage divider, factor ~6.07 | `BLADE_BATTERY_PIN` |
| 2 | Output | Onboard LED | Heartbeat/Status | `LED_BUILTIN` |
| 4 | Output | Blade ESC PWM | 1000-2000μs servo signal | `BLADE_ESC_PIN` |
| 5 | Output | Blade Relay | Safety cutoff (active HIGH) | `BLADE_RELAY_PIN` |
| 10 | Input | Button 1 (Test Fwd) | Pullup, Speed 200 | `BUTTON1_PIN` |
| 11 | Input | Button 2 (Test Rev) | Pullup, Speed -200 | `BUTTON2_PIN` |
| 13 | I2C | MPU6050 SDA | Motion sensor data | `Wire.begin(13, 14)` |
| 14 | I2C | MPU6050 SCL | Motion sensor clock | `Wire.begin(13, 14)` |
| 15 | Input | E-Stop Button | Pullup, Physical emergency stop | `EMERGENCY_STOP_PIN` |
| 17 | UART TX | To Hoverboard | Green wire (RX) | `HOVER_TX_PIN` |
| 18 | UART RX | From Hoverboard | Blue wire (TX) | `HOVER_RX_PIN` |
| 19 | Output | Blade Unit Power | Controls blade power relay ⚠️ | `BLADE_UNIT_PIN`¹ |
| 20 | Output | Drive Unit Power | Controls drive power relay | `DRIVE_UNIT_PIN` |
| 35 | Output | Blade UP | Lift actuator up | `BLADE_UP_PIN` |
| 36 | Output | Blade DOWN | Lift actuator down | `BLADE_DOWN_PIN` |
| 46 | Input | MPU6050 INT | Data ready interrupt (DMP) | Interrupt Pin |

> **✅ Fix:** `BLADE_UNIT_PIN` wurde korrigiert (war fälschlicherweise 20, jetzt 19). Siehe `docs/archive/changes/2026-03-15_blade-unit-pin-fix.md`

---

## SECTION 2: ESC PROGRAMMING GUIDE

### 2.1 Programming Logic ("Stick Programming")
1. **ENTER MENU:** Send "BLADE MAX" (PWM 2000), Power on ESC
2. **NAVIGATING:** Motor beeps Parameter Number in loop
3. **SELECTING:** Move Throttle to NEUTRAL (PWM 1500)
4. **CHANGING:** Toggle Throttle MAX → NEUTRAL
5. **SAVING:** Wait for confirmation tone, disconnect power

### 2.2 Parameter Table

| # | Beep Code | Function | Values |
|---|-----------|----------|--------|
| 1 | 1 Short | Operation Model | **Fwd/Brake*** |
| 2 | 2 Short | Motor Direction | **Normal*** |
| 3 | 3 Short | Start Mode | **Lvl5 (Rec)** |
| 12 | 2 Long, 2 Short | Lithium Saving | **Auto*** |
| 17 | 3 Long, 2 Short | Sync Rectification | **Open (On)** |

---

## SECTION 3: COMMAND REFERENCE

### 3.1 Serial Monitor Commands

| Action | Command | Example |
|--------|---------|---------|
| Drive | `steer,speed` | `100,200` |
| Stop | `0,0` | `0,0` |
| Reset | `reset` | `reset` |
| Skill | `[ID]` | `3` |
| Skill+Param | `[ID], [Value]` | `16, 607` |

### 3.2 Skill List

| Skill | Function | Parameters |
|-------|----------|------------|
| 1 | System Reset | - |
| 2 | 90° Turn | 1=Right, -1=Left |
| 3 | Hold the Line | - |
| 4 | 180° Turn | 1=Right, -1=Left |
| 5 | Blade Lift | 1=Up, -1=Down |
| 6 | Blade Power | 1=ON, -1=OFF |
| 7 | Set Limits | Steer, Speed |
| 8 | E-Stop | 20=Trigger, -20=Reset |
| 14 | Capsize Params | Angle, Timeout |
| 15 | PD Gains | Kp, Kd |
| 16 | Blade Battery Factor | Factor × 100 |
| 17 | Drive Battery Factor | Factor × 1000 |
| 20 | Distance Mission | DistL, DistR, SpeedL, SpeedR, Accel, Brake |

### 3.3 JSON Commands (WebSocket)

```json
// Drive Joystick
{ "type": "joystick", "speed": 400, "steer": -200 }

// Execute Skill
{ "type": "command", "data": { "code": 2, "value": -1 } }
```

---

## SECTION 4: REMOTE CONTROLLER (PS3)

### 4.1 Driving Controls
- **Left Stick (Vertical):** Acceleration/Braking/Reverse
- **Right Stick (Horizontal):** Steering
- **L3/R3:** Cruise Control

### 4.2 Function Keys
- **L1:** EMERGENCY STOP
- **R1:** Blade ON
- **R2:** Blade OFF
- **Triangle:** Hold the Line (Skill 3)
- **Cross (X):** SYSTEM RESET (Skill 1)

---

## SECTION 5: SAFETY SYSTEMS

### 5.1 Capsize Protection (Skill 8)
- **Trigger:** Pitch/Roll > 60° for > 1500ms
- **Action:** Hard Stop + Blade Relay CUT

### 5.2 Voltage Protection

| System | Level | Voltage | Action |
|--------|-------|---------|--------|
| Drive | Critical | < 31.0V | Shutdown |
| Drive | Safety | < 32.5V | Half Speed |
| Drive | High | > 43.0V | E-Stop |
| Blade | Critical | < 12.0V | Relay Cut |
| Blade | Safety | < 13.0V | Safety Mode |

### 5.3 WiFi Configuration
- **Channel:** 6
- **HTTP Port:** 80
- **AP-IP:** 192.168.4.1
- **SSID:** Lawnmower_Control
- **Password:** 123456789

---

## SECTION 6: Skill 20 - Distance Mission

**Format:** `20, DistL, DistR, SpeedL, SpeedR, Accel, Brake`

| Parameter | Range | Unit | Description |
|-----------|-------|------|-------------|
| DistL/R | -32000...32000 | cm | Distance per wheel |
| SpeedL/R | 0...1000 | - | Max speed (0 = use currentMaxSpeed) |
| Accel | 10...500 | % | Acceleration ramp |
| Brake | 10...500 | % | Braking ramp |

**Examples:**
```
20, 100, 100, 200, 200, 20, 20  -> 1m straight, 20% ramps
20, 95, -95, 200, 200, 20, 20   -> 90° point turn
```

---

> **This document is the "SINGLE SOURCE OF TRUTH" for hardware specifications.**  
> **For software architecture:** See `AGENTS.md`  
> **For change history:** See `docs/archive/`
