# 🔧 CHANGE-LOG: GPIO Pinout Korrektur

> **Datum:** 2026-03-15  
> **Agent:** Kimi Code CLI  
> **Typ:** Dokumentations-Korrektur + Bug-Identifikation

---

## 🎯 ZUSAMMENFASSUNG

**Problem:** GPIO-Pinout in MD-Dateien war inkonsistent mit tatsächlicher Code-Implementierung

**Lösung:** 
1. PIN-Definitionen aus `src/main.cpp` extrahiert (Zeilen 126-143, 1682)
2. Alle MD-Dateien synchronisiert
3. Kritischen Code-Bug identifiziert (BLADE_UNIT_PIN = DRIVE_UNIT_PIN = 20)

---

## 🔍 QUELLENANALYSE (src/main.cpp)

### Extrahierte Pin-Definitionen

```cpp
// Zeilen 137-143: Pin-Definitionen
#define BLADE_UNIT_PIN               20        // Power Pin für Blade Unit  ← BUG!
#define DRIVE_UNIT_PIN               20        // Power Pin für Drive Unit    ← BUG!
const int BLADE_ESC_PIN = 4;                  // Wunschgemäß auf Pin 4
const int BLADE_RELAY_PIN = 5;

// Zeilen 126-127: UART
#define HOVER_RX_PIN             18          // GPIO for HoverSerial TX blue cable
#define HOVER_TX_PIN             17          // GPIO for Hoverserial RX green cable

// Zeilen 131-136: Sonstige
#define BUTTON1_PIN              10
#define BUTTON2_PIN              11
#define BLADE_UP_PIN               35
#define BLADE_DOWN_PIN           36
#define EMERGENCY_STOP_PIN       15
#define BLADE_BATTERY_PIN            1

// Zeile 1682: MPU6050 Interrupt
pinMode(46, INPUT); // GPIO 46 als Eingang für den INT-Pin
```

### Konsistenzprüfung via Kommentare

| Zeile | Kommentar | Implizierter PIN |
|-------|-----------|------------------|
| 1526 | "Pin 19 auf LOW beim Start" | BLADE_UNIT_PIN = 19 |
| 1529 | "Pin 20 auf LOW beim Start" | DRIVE_UNIT_PIN = 20 |
| 896 | "Skill 6: Blade Unit ON (Pin 19 HIGH)" | BLADE_UNIT_PIN = 19 |

**→ Code-Defekt bestätigt:** Beide sind als 20 definiert, sollten aber 19 und 20 sein.

---

## 📋 KORREKTUREN AN MD-DATEIEN

### 1. AGENTS.md - GPIO Pinout Tabelle

**Vorher:** Fehlende Pins 19, 20, 35, 36, 46

**Nachher:** Vollständige Tabelle mit allen 16 Pins:

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

### 2. TECHNICAL_MANUAL.md

**Hinzugefügt:** Section 1.4 Pinout Reference - Vollständige Liste

### 3. copilot-instructions.md

**Korrigiert:** 
- ~~`src/ManualLawnmover.cpp`~~ → `docs/TECHNICAL_MANUAL.md`

---

## 🐛 BUG-REPORT: BLADE_UNIT_PIN

**ID:** BUG-001  
**Schweregrad:** 🔴 KRITISCH (Hardware-Konflikt)  
**Status:** Dokumentiert, Code-Fix ausstehend

### Beschreibung
Beide Power-Control-Pins sind auf GPIO 20 gemappt:

```cpp
#define BLADE_UNIT_PIN               20  
#define DRIVE_UNIT_PIN               20  // Gleiche Nummer!
```

### Auswirkung
- Blade-Unit und Drive-Unit können nicht unabhängig gesteuert werden
- Beide Relais reagieren auf dieselbe GPIO-Leitung
- Hardware-Konflikt auf dem PCB wenn beide angeschlossen

### Empfohlener Fix
```cpp
#define BLADE_UNIT_PIN               19  // Korrigiert: War 20
#define DRIVE_UNIT_PIN               20  // Bleibt 20
```

### Verifikation im Code
Die Kommentare in Zeile 1526 und 1529 bestätigen die beabsichtigte Zuweisung:
- 1526: "Pin 19 auf LOW beim Start" (für BLADE_UNIT_PIN)
- 1529: "Pin 20 auf LOW beim Start" (für DRIVE_UNIT_PIN)

---

## 📝 METADATEN

| Feld | Wert |
|------|------|
| Dateien geändert | 3 MD-Dateien |
| Code-Änderungen | 0 (nur Dokumentation) |
| Neue Archive-Dateien | 4 |
| Bugs identifiziert | 1 (kritisch) |
| Build-Status | ✅ OK |

---

## 🔗 VERKNÜPFUNGEN

- Siehe auch: `docs/archive/tree/2026-03-15_tree.md`
- Siehe auch: `AGENTS.md` (GPIO Pinout Sektion)
- Siehe auch: `docs/TECHNICAL_MANUAL.md` (Section 1.4)
