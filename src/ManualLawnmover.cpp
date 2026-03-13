#include <Arduino.h>

/*
// =================================================================================================
// ========================= LAWNMOWER MASTER TECHNICAL MANUAL (v5.0) ==============================
// =================================================================================================
// Partition Scheme ändern bei Installation: Wähle in der Arduino IDE "Huge APP (3MB No OTA/1MB SPIFFS)" or "Minimal SPIFFS (1.9MB APP with OTA)". 
// Dies gibt deinem Code mehr als doppelt so viel Platz (3,0 MB statt 1,2 MB).
// =================================================================================================
// OTA (per Wifi Code Upload) upload Password über die Arduino.IDE lautet: "1234"
// -------------------------------------------------------------------------------------------------
// -- SECTION 1: HARDWARE & ELECTRONICS SPECIFICATIONS
// -------------------------------------------------------------------------------------------------
//
// 1.1 CORE COMPUTING
//    - Microcontroller: ESP32-S3 Dev Kit
//    - IMU Sensor: MPU6050 (I2C Address 0x68).
//      - Wiring: SDA -> GPIO 13, SCL -> GPIO 14.
//      - Interrupt: INT -> GPIO 46 (Essential for DMP stability).
//
// 1.2 DRIVE SYSTEM (HOVERBOARD)
//    - Motors: 2x Brushless Hub Motors (350W+ each).
//    - Controller: Hoverboard Mainboard with UART firmware.
//    - Batterietyp: Samsung 35E (Li-Ion)
//    - Konfiguration: 10S2P 
//    - Nennspannung: 36,0 V
//    - Max. Ladespannung: 42,0 V
//    - Nennkapazität (Block): 17,5 Ah (2 x 3500mAh)
//    - BMS Modell: Daly Li-Ion 10S 36V
//    - Dauerentladestrom (BMS): 20 A (Zellen 2x8A)
//    - Max. Entladestrom (Zellen): 26 A (2 x 13A Peak)
//    - Entladeschlussspannung: 26,5 V (Block Zellen) (BMS 27V)
//    - BMS Überladungsschutz: 4,25V ±0,05V (pro Zelle)
//    - BMS Tiefentladeschutz: 2,7V ±0,05V (pro Zelle)
//    - Balance-Startspannung: 4,18 V
//    - Betriebstemperatur: -20°C bis 60°C
//    - Communication: UART Serial (115200 Baud).
//      - ESP32 TX (GPIO 17) -> Hoverboard GREEN Wire (RX).
//      - ESP32 RX (GPIO 18) -> Hoverboard BLUE Wire (TX).
//    - Voltage Correction Factor (display faktor adjustable in webpage).
//
// 1.3 BLADE SYSTEM (THE CUTTER)
//    - Motor: SURPASS-HOBBY C3548-V2 (Outrunner)
//      - Spec: 900KV, 14 Poles, 1000W Max.
//      - Voltage Range: 7-18V (2S-4S LiPo).
//      - Max Current: 60A.
//      - Props: 11x8, 13x6, 14x7.
//    - ESC: 120A Brushless ESC (T/XT60 Plug)
//      - SBEC: 5.8V / 3A.
//      - Support: 2-4S LiPo.
//    - Batterietyp: Samsung 35E (Li-Ion)
//    - Konfiguration: 4S6P (Vorschlag basierend auf 40A Last)
//    - Nennspannung: 14,4 V
//    - Max. Ladespannung: 16,8 V
//    - Nennkapazität (Block): 21,0 Ah (6 x 3500mAh)
//    - BMS Modell: Daly Li-Ion 4S 12V 40A
//    - Dauerentladestrom (BMS): 40 A (Zellen 48A)
//    - Max. Entladestrom (Zellen): 48 A (6 x 8A Peak)
//    - Entladeschlussspannung: 10,6 V (Block) (BMS 10,8V)
//    - BMS Überladungsschutz: 4,25V ±0,05V (pro Zelle)
//    - BMS Tiefentladeschutz: 2,7V ±0,05V (pro Zelle)
//    - Balance-Startspannung: 4,18 V
//    - Betriebstemperatur: -20°C bis 60°C
//    - Control:
//      - ESC Signal: GPIO 4 (Servo PWM: 1000us = Stop, 2000us = Full).
//      - Safety Relay: GPIO 5 (Active HIGH = Relay ON/Power Enabled).
//
// 1.4 VOLTAGE MEASUREMENT (BLADE SYSTEM)
//    - Sensor Pin: GPIO 1 (Analog ADC).
//    - Hardware implementation: Voltage Divider.
//      - Resistor Side A: 5x 220 Ohm (Series or combo) approx 1.1k Ohm.
//      - Resistor Side B: ~5k Ohm range.
//    - EMPIRICAL CALIBRATION (The "Truth"):
//      - Input Voltage: 15.00 V
//      - Measured at Pin: 2.47 V
//      - Calculation Factor: on website adjustable for correct display
//      - Settings Value: `bladeBatteryFactor` should be set to approx 6.4.
//
// 1.5 PINOUT REFERENCE TABLE
//    | GPIO | Type   | Function                          | Details |
//    |------|--------|-----------------------------------|---------|
//    | 1    | Analog | Blade Battery Sense               | Factor 6.07 |
//    | 2    | Output | Onboard LED                       | Heartbeat/Status |
//    | 4    | Output | Blade ESC PWM                     | 1000-2000us |
//    | 5    | Output | Blade Relay                       | Safety Cutoff |
//    | 10   | Input  | Button 1 (Test Fwd)               | Pullup, Speed 200 |
//    | 11   | Input  | Button 2 (Test Rev)               | Pullup, Speed -200 |
//    | 13   | I2C    | MPU6050 SDA                       | Motion Sensor |
//    | 14   | I2C    | MPU6050 SCL                       | Motion Sensor |
//    | 15   | Input  | E-Stop Button                     | Pullup, Physical Emergency |
//    | 17   | UART   | TX to Hoverboard                  | Green Wire |
//    | 18   | UART   | RX from Hoverboard                | Blue Wire |
//    | 46   | Input  | MPU6050 INT                       | Data Ready Interrupt |
//
// -------------------------------------------------------------------------------------------------
// -- SECTION 2: ESC PROGRAMMING GUIDE (120A BRUSHLESS)
// -------------------------------------------------------------------------------------------------
//
// 2.1 PROGRAMMING LOGIC ("STICK PROGRAMMING")
//    Since there is no card, we use the motor beeps.
//    - ENTER MENU:
//      1. Turn on ESP32/Remote. Send "BLADE MAX" (PWM 2000).
//      2. Power on ESC (Connect Battery).
//      3. Wait for "Musical Tune" (Self-check) -> Wait for Loop Beeps.
//    - NAVIGATING:
//      The motor beeps the Parameter Number in a loop (e.g., "Beep-Beep" = Param 2).
//      Long Beep = 5. (e.g., Long + Short = 6).
//    - SELECTING PARAMETER:
//      When you hear the code for the parameter you want (e.g., 3 beeps for Start Mode),
//      immediately move Throttle to NEUTRAL (PWM 1500).
//    - CHANGING VALUE:
//      The motor now beeps the CURRENT VALUE code.
//      To change, toggle Throttle to MAX, then back to NEUTRAL.
//    - SAVING:
//      Wait for the special confirmation tone, then disconnect power.
//
// 2.2 PARAMETER TABLE & BEEP CODES
//    (Based on 120A Manual. Defaults marked with *).
//
//    | #  | Beep Code (Loop) | Function               | Values (Beeps indicate Option #) |
//    |----|------------------|------------------------|-----------------------------------------------------------|
//    | 1  | 1 Short          | Operation Model        | 1: **Forward/Brake***, 2: Fwd/Rev/Brk, 3: Fwd/Rev |
//    | 2  | 2 Short          | Motor Direction        | 1: **Normal***, 2: Reverse |
//    | 3  | 3 Short          | Start Mode (Punch)     | 1: Lvl1, 2: Lvl2, 3: Lvl3*, 4: Lvl4, 5: **Lvl5 (Rec)** |
//    | 4  | 4 Short          | Min Fwd Strength       | 1: 5%*, 2: 10%, 3: 15% ... |
//    | 5  | 1 Long           | Min Rev Strength       | 1: 5%*, 2: 10% ... |
//    | 6  | 1 Long, 1 Short  | Max Rev Strength       | 1: 25%, 2: 50%, 3: 75%, 4: 100%* |
//    | 7  | 1 Long, 2 Short  | Initial Brake (Drag)   | 1: 0%*, 2: 5% ... |
//    | 8  | 1 Long, 3 Short  | Max Brake Strength     | 1: 50%, 2: 100%* ... |
//    | 9  | 1 Long, 4 Short  | Braking Force          | 1: 25%, 2: 50%* ... |
//    | 10 | 2 Long           | Neutral Range          | 1: 2%, 2: 3%*, 3: 4% ... |
//    | 11 | 2 Long, 1 Short  | Brake Frequency        | 1: 16KHz*, 2: 8KHz ... |
//    | 12 | 2 Long, 2 Short  | Lithium Saving         | 1: **Auto*** (Use this or Non-Prot for 10S!) |
//    | 13 | 2 Long, 3 Short  | Low Volt Cutoff        | 1: 2.6V, 2: 2.8V, 3: 3.0V*, 4: 3.2V, 5: 3.4V |
//    | 14 | 2 Long, 4 Short  | Low Volt Protection    | 1: Ignore, 2: **Reduce Power*** |
//    | 15 | 3 Long           | Rev Max Throttle       | (Timing settings) |
//    | 16 | 3 Long, 1 Short  | Fwd Max Throttle       | (Timing settings) |
//    | 17 | 3 Long, 2 Short  | Sync Rectification     | 1: Close (Off), 2: **Open (On)** (Recommended) |
//
//    ** RECOMMENDED FOR CABLE RESET **
//    - Parameter 3 (Start Mode): Set to Option 5 (Level 5). We need maximum torque "jerk" to extend the line.
//    - Parameter 17 (Sync Rect): Set to Open (Enabled) for better active braking and efficiency.
//
// -------------------------------------------------------------------------------------------------
// -- SECTION 3: COMMAND REFERENCE & DATA EXAMPLES
// -------------------------------------------------------------------------------------------------
//
// 3.1 SERIAL MONITOR COMMANDS (ESP32)
//    You can control the mower directly by typing these into the Arduino or also Web Serial Monitor.
//
//    | Action | Serial Command Syntax | Example | Details |
//    |--------|-----------------------|---------|---------|
//    | Drive | `steer,speed` | `100,200` | Steer 100 (Right), Speed 200 (Fwd). |
//    | Stop | `0,0` | `0,0` | Immediate stop via joystick logic. |
//    | Reset | `reset` | `reset` | Reboots the ESP32. |
//    | Skill | `[ID]` | `3` | Activates Skill 3 (Hold Line). |
//    | Skill+Param | `[ID], [Value]` | `16, 607` | Sets Blade Battery Factor to 6.07. |
//    | Skill 7 | `7, speed, steer` | `7, 800, 600` | Sets Max Speed to 800, Max Steer to 600. |
//    | Skill 2 (Right) | `2+` | `2+` | Triggers 90° Turn Right. |
//    | Skill 2 (Left) | `2-` | `2-` | Triggers 90° Turn Left. |
//    | Blade On | `6` | `6` | Toggles Blade (Param 1). |
//
// 3.2 JSON COMMANDS (WEBSOCKET/WIFI)
//    These are sent by the Web Interface.
//
//    - **Drive Joystick:**
//      `{ "type": "joystick", "speed": 400, "steer": -200 }`
//
//    - **Execute Skill 2 (90° Turn Left):**
//      `{ "type": "command", "data": { "code": 2, "value": -1 } }`
//
//    - **Set Capsize Parameters (Skill 14):**
//      `{ "type": "command", "data": { "code": 14, "angle": 55, "timeout": 1000 } }`
//      <Setzt den Auslösewinkel (°) und das Timeout (ms) für die Kipp-Schutz-Logik. Werte werden dauerhaft gespeichert.>
//
//    - **Blade Cable Reset (Skill 11):**
//      `{ "type": "command", "data": { "code": 11, "value": 1800 } }`
//
// Grundlage der Berechnung im Speed Mode Robot, neue config: derzeit aktiv!!
//
// ESP Modulation: modSpeed=u Speed+(u Steer/2) | modSteer=u Steer /2 .
// Hoverboard Mixer: L=modSpeed+modSteer | R=modSpeed-modSteer.
// Nr.,"Input (Steer, Speed)","Modulated (S, V)",Rad Links (L),Rad Rechts (R),Verhalten
// 1,"0, 200","0, 200",200,200,Geradeausfahrt (Normal) 
// 2,"200, 200","100, 300",400,200,Deine Wunsch-Kurve (Rechts) 
// 3,"-200, 200","-100, 100",0,200,Enge Linkskurve (L-Rad steht) 
// 4,"200, 0","100, 100",200,0,Drehung Rechts (R-Rad steht) 
// 5,"-200, 0","-100, -100",-200,0,Drehung Links rückwärts 
// 6,"0, -200","0, -200",-200,-200,Rückwärtsfahrt geradeaus 
// 7,"100, 200","50, 250",300,200,Leichte Rechtskurve 
// 8,"400, 400","200, 600",800,400,Schnelle Rechtskurve 
// 9,"200, -200","100, -100",0,-200,Rückwärts-Linkskurve 
// 10,"0, 0","0, 0",0,0,Stillstand (E-Stop/Neutral) 
//
// 3.3 FULL SKILL LIST (THE "API")
//    - **Skill 1:** System Reset (Clears Gyro, Skills, E-Stop).
//    - **Skill 2:** 90° Turn (Param: 1=Right, -1=Left).
//    - **Skill 3:** Hold the Line (Maintains heading via PD).
//    - **Skill 4:** 180° Turn (Param: 1=Right, -1=Left).
//    - **Skill 5:** Blade Lift (Servo) (1=Up, -1=Down).
//    - **Skill 6:** Blade Power (1=ON, -1=OFF).
//    - **Skill 7:** Set Limits (Param 2=Steer, Param 3=Speed).
//    - **Skill 8:** E-Stop Logic.
//       - Value `20`: **TRIGGER E-STOP** (System Locks).
//       - Value `-20`: **RESET E-STOP** (Unlock System).
//       - Value `1`: Enable Capsize.
//       - Value `-1`: Disable Capsize.
//    - **Skill 14:** Set Capsize Parameters (Angle, Timeout).
//       - Beispiel: `{ "type": "command", "data": { "code": 14, "angle": 60, "timeout": 1500 } }`
//       - Setzt den Auslösewinkel (°) und das Timeout (ms) für die Kipp-Schutz-Logik. Werte werden dauerhaft gespeichert.
//    - **Skill 9:** ESP32 Reboot.
//    - **Skill 10:** Set Blade Speeds (Eco, Stage2, Max, Reset).
//    - **Skill 11:** Execute Blade Cable Reset (Ramps up motor to extend line).
//    - **Skill 12:** Set Drive Voltage/Temp Limits.
//    - **Skill 13:** Set Blade Voltage/Temp Limits.
//    - **Skill 14:** Set Capsize Parameters (Angle, Timeout).
//    - **Skill 15:** Set PD Gains (Steering stabilizer).
//    - **Skill 16:** Set Blade Battery Factor (Calibration).
//    - **Skill 999:** Web-Serial Command Bridge (Value = Raw String / Terminal).
//    - **Skill 255:** Heartbeat (Keep-alive).
//
// -------------------------------------------------------------------------------------------------
// -- SECTION 4: REMOTE CONTROLLER MAPPING (PS3)
// -------------------------------------------------------------------------------------------------
//
// 4.1 DRIVING CONTROLS
//    - **Left Stick (Vertical):** Acceleration / Braking / Reverse.
//    - **Right Stick (Horizontal):** Steering.
//    - **L3 (Stick Press):** Cruise Control (Speed).
//    - **R3 (Stick Press):** Cruise Control (Steering).
//    - **D-Pad Up/Down:** Speed Limit Mode (1=Slowest, 4=Fastest).
//    - **D-Pad Left/Right:** Steering Trim (Fine tune straight line).
//
// 4.2 FUNCTION KEYS
//    - **L1:** EMERGENCY STOP (Deadman or Toggle depending on config).
//    - **R1:** Blade ON.
//    - **R2:** Blade OFF.
//    - **Triangle:** Hold the Line (Skill 3).
//    - **Square:** 90° Turn Prep/Execute.
//    - **Circle:** 180° Turn Prep/Execute.
//    - **Cross (X):** SYSTEM RESET (Skill 1). Use this to clear E-Stops or weird gyro states.
//    - **Start:** Reboot ESP32.
//    - **Select:** Change Local Remote Mode (Linear vs Curves).
//
// -------------------------------------------------------------------------------------------------
// -- SECTION 5: SAFETY SYSTEMS & TROUBLESHOOTING
// -------------------------------------------------------------------------------------------------
//
// 5.1 CAPSIZE PROTECTION (Skill 8)
//    - The MPU6050 monitors orientation 100 times a second.
//    - Trigger: If Pitch or Roll > `capsizeAngle` (Default 60°) for > `capsizeTimeout` (Default 1500ms).
//    - Action: Immediate Hard Stop (0,0), Blade Relay CUT, Safety Lock active.
//    - Recovery: You MUST send a "Stop/Reset" command (Web button or PS3 'X') to drive again.
//
// 5.2 VOLTAGE PROTECTION (BATTERY SAVER)
//    - **Drive System:**
//      - < 31.0V: Critical Shutdown (Stop).
//      - < 32.5V: Safety Mode (Half Speed, Blade Forced OFF).
//      - > 43.0V: High Voltage E-Stop (Regen breaking protection).
//      - BMS Cutoff: 26,5 V (Hardware-Sicherung).
//    - **Blade System:**
//      - < 12.0V: Shutdown (Relay Cut).
//      - < 13.0V: Safety Mode.
//      - BMS Cutoff: 10,6 V (Hardware-Sicherung).
//      - Note: Values scaled by `bladeBatteryFactor`.
//
// 5.3 TEMPERATURE
//    - Drive Board > 50°C: E-Stop.
//    - Drive Board < 1°C: E-Stop (Frost protection).
//
// 5.4 WIFI / AP BEHAVIOR
//     - WiFi Channel: 6 | HTTP Port: 80 | AP-IP: 192.168.4.1 | STA-IP: 192.168.178.123
//    - Startup: Creates AP "Lawnmower_Control" (Pass: 123456789).
//    - Connection: Tries to connect to stored Home WiFi credentials.
//    - Timeout: If Home WiFi connected, AP turns OFF after 5 minutes to save power/interference.
//    - Recovery: If Home WiFi is lost, AP restarts automatically.
//
// Platzsparende technische Beschreibung der Web-UI für die AI
// ### UI CAPABILITY MAP (FULL) ###
// TAB [Manual Control]:
// - JOYSTICK: Analog Drive (Steer/Speed), Auto-Stop on release.
// - BUTTONS: 'Hold Line' (Skill 3), 'Stop Reset' (Skill 8, Val -20).
// - TURNS: 90/180 Grad (Left/Right) via Skill 2/4.
// - BLADE: Toggle ON (Skill 6, Val 1), OFF (Skill 6, Val -1).
// - BLADE SPEEDS: 'Speed 1' (Eco), 'Speed 2' (Working) via Skill 6 + RPM-Logic.
// - RESET: 'Blade cable length reset' (Skill 11).
//
// TAB [Lawnmower Setup]:
// - SAFETY: Emergency Stop (ON/OFF), Capsize Logic (ON/OFF).
// - DRIVE LIMITS: Input 'Max Speed', 'Max Steer' (Skill 7).
// - PD-GAINS: Input 'Kp', 'Kd' (Skill 15).
// - DRIVE SYSTEM LEVELS: Shutdown/Safety/High Voltage & Temp-Limits (Skill 12).
// - BLADE SYSTEM LEVELS: Shutdown/Safety/High Voltage & Temp-Limits (Skill 13).
// - BLADE SPEEDS SETUP: Eco/Working/Max PWM, Cable Reset RPM/Ramp/Duration (Skill 10).
// - CAPSIZE SETUP: Angle (°), Timeout (ms) (Skill 14).
//
// TAB [AI Assistant]:
//     - CHAT: Interaktiver Support via Gemini API basierend auf Manual & Live-Werten.
//     - WEB-SERIAL: Direktes Senden von Commands an ESP32 (Skill 999 Interface).
//
// TAB [WiFi Setup]:
// - WIFI: SSID/Password input, 'Connect WiFi', 'Reset Device', 'Reset WiFi Credentials'.
// - INFO: Direct-AP: LawnmowerControl (123456789).
//
// LIVE MONITORING:
// - Drive/Blade: V, Temp, Battery % (Graphic), Serial Monitor Log (Bottom).
//
// -------------------------------------------------------------------------------------------------
// -- SECTION 6: SYSTEM UPDATES & DYNAMIC CALIBRATION (v5.1)
// -------------------------------------------------------------------------------------------------
//
// 6.1 LIVE DIAGNOSTICS & WEB-LOG (Skill 998)
//    - Function: Real-time mirroring of Serial Debug data to the Web-UI.
//    - Activation: Via Web-UI or Command Code 998 (Value 1 = ON, 0 = OFF).
//    - Transmission: Buffered packet-send every 500ms to ensure WiFi/Task stability.
//    - Buffer: 2000-character limit with auto-flush/overflow protection.
//
// 6.2 DYNAMIC SENSOR CALIBRATION (Faktor-Adjustment)
//    - Drive Factor (Skill 17): Adjusts Hoverboard battery readout.
//      - Syntax: 17, [Value] (Value = Factor * 1000).
//      - Default: 938 (0.938) for 37.0V reference.
//    - Blade Factor (Skill 16): Adjusts ADC readout for 4S/10S Blade battery.
//      - Syntax: 16, [Value] (Value = Factor * 100).
//      - Default: 640 (6.40) for 15.0V/2.47V reference.
//    - Persistence: Values saved to EEPROM via `saveSettings()` after update.
//
// 6.3 ADVANCED STEERING CONTROL (PD-Regler)
//    - Logic: Replaced static steering with Proportional-Derivative (PD) control.
//    - Gains (Skill 15): Set Kp and Kd for heading stability.
//      - Syntax: 15, [Kp*100], [Kd*100].
//      - Default: Kp=4.0 (400), Kd=1.5 (150).
//    - Application: Affects Skill 2 (90°), Skill 3 (Hold Line), and Skill 4 (180°).
//
// 6.4 WEB-SERIAL COMMAND BRIDGE (Skill 999)
//    - Interface: Direct terminal input in the AI/Web-UI.
//    - Parsing:
//      - Single Integer (e.g., "3"): Executes `triggerSkill(3)`.
//      - Comma Pair (e.g., "0,200"): Executes `Send(steer, speed)`.
//    - Safety: Inputs are constrained by `maxSpeed` and `maxSteer` settings.
//
// ====================================================================================
// SKILL 20 - DISTANCE MISSION PARAMETERS (Binary web serial monitor Interface)
// Command Format to type into web serial interface of the esp32 to elaborate and later forward to Hoverboard via UART: 20, DistL, DistR, SpeedL, SpeedR, Accel, Brake
// ====================================================================================
// Parameter | Bereich       | Einheit | Erklärung
// ----------|---------------|---------|----------------------------------------------
// DistL/R   | -32000...32000| cm      | Distanz pro Rad (Positiv=Vor, Negativ=Zurück)
// SpeedL/R  | 0...1000      | einheit     | Max. Geschwindigkeit (0 = nutzt currentMaxSpeed)
// Accel     | 10...500      | %   | Anfahrbereich in % zur Distanz
// Brake     | 10...500      | %   | Bremsbereich in % zur Distanz
// ====================================================================================
// BEISPIELE (Eingabe über Web-Serial Code 999):
// 20, 100, 100, 200, 200, 20, 20 -> 1m geradeaus, 20% Rampe (Anfahren/Bremsen)
// 20, 500, 500, 300, 300, 10, 15 -> 5m Fahrt, kurzes Anfahren, 15% Bremsweg
// 20, 95, -95, 200, 200, 20, 20 -> 90 Grad Drehung (Punkt-Wende)
// ====================================================================================
// =================================================================================================
//
//  !!! SYSTEM PROMPT FOR AI ASSISTANT !!!
//  This document is the "SINGLE SOURCE OF TRUTH". It contains every hardware spec, software command, 
//  ESC beep code, and safety protocol for the Lawnmower.
//  Use this to answer queries about wiring, programming, serial debugging, and error codes.
//
// =================================================================================================
*/