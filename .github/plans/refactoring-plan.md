# Implementierungsplan: ESP32 Lawnmower Refactoring

**Status:** Plan Mode Aktiv  
**Erstellt:** 2026-03-13  
**Ziel:** Umsetzung aller validen Punkte aus To-do1.md

---

## Übersicht der Tasks

| Phase | Task | Priorität | Status |
|-------|------|-----------|--------|
| 1 | Task 1.2: Task Watchdog (WDT) | Hoch | ⬜ Geplant |
| 1 | Task 1.3: SerialFeedback Sicherheit | Hoch | ⬜ Geplant |
| 1 | Task 1.4: E-Stop vs. Mission | Hoch | ⬜ Geplant |
| 2 | Task 2.1: Send() Refactoring | Hoch | ⬜ Geplant |
| 2 | Task 2.2: Variable Shadowing | Hoch | ⬜ Geplant |
| 2 | Task 2.3: Typen-Konflikte | Mittel | ⬜ Geplant |
| 3 | Task 3.2: EEPROM Initialisierung | Mittel | ⬜ Geplant |
| 3 | Task 3.3: Web-Command ACK | Mittel | ⬜ Geplant |
| 4 | Task 4.1: Doppelte Hardware-Deklarationen | Niedrig | ⬜ Geplant |
| 4 | Task 4.2: Template Overloads | Niedrig | ⬜ Geplant |
| 4 | Task 4.3: triggerSkill Refactoring | Niedrig | ⬜ Geplant |

---

## ❌ NICHT UMSETZEN (Fehlerhafte Kritik)

| Task | Begründung |
|------|------------|
| Task 1.1: I2C Race Conditions | `analogRead()` stört I2C NICHT - technisch falsch |
| Task 3.1: JSON Stack Overflow | 512 Bytes auf Stack sind für ESP32-S3 unkritisch |

---

## Detaillierte Implementierungspläne

---

### 🚨 PHASE 1: Kritische Fehler & Stabilität

---

#### Task 1.2: Task Watchdog (WDT) Triggers verhindern

**Problem:** `connectToWiFi()` hat eine `while`-Schleife ohne `vTaskDelay()` → WDT Reset

**Datei:** `src/main.cpp`

**Aktueller Code (ca. Zeile 2718):**
```cpp
while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    vTaskDelay(500 / portTICK_PERIOD_MS);  // <- BEREITS VORHANDEN!
    if (ENABLE_DEBUG_SERIAL) {
      debugPrint(".");
    }
}
```

**Analyse:** Das `vTaskDelay(500)` ist bereits vorhanden! Keine Änderung nötig.

**Aber:** Prüfe `handleSerialInput()` auf fehlende Delays (falls vorhanden).

**Status:** ⬜ Zu prüfen ob wirklich ein Problem existiert

---

#### Task 1.3: SerialFeedback Struct Sicherheit (Buffer Overflow)

**Problem:** `memcpy(&Feedback, &NewFeedback, sizeof(SerialFeedback))` ohne Längenprüfung

**Datei:** `src/main.cpp` - Funktion `Receive()`

**Aktueller Code analysieren:**
```cpp
// Suche nach Receive() Funktion und memcpy
```

**Implementierung:**
1. Prüfung `if (sizeof(SerialFeedback) == 18)` in `setup()` einbauen
2. Guard in `Receive()` einbauen: `if (idx == sizeof(SerialFeedback))`

**Code-Änderung:**
```cpp
// In setup() hinzufügen:
if (sizeof(SerialFeedback) != 18) {
    debugPrintln("FATAL: SerialFeedback struct size mismatch!");
    // Oder assert/while(1) für Entwicklung
}

// In Receive() vor memcpy:
if (idx == sizeof(SerialFeedback)) {
    memcpy(&Feedback, &NewFeedback, sizeof(SerialFeedback));
}
```

**Status:** ⬜ Geplant

---

#### Task 1.4: Konfliktlösung E-Stop vs. Mission (Skill 20)

**Problem:** E-Stop (Skill 8) hat keinen Vorrang während Skill 20 läuft

**Datei:** `src/main.cpp` - `controlLogicTask()` ca. Zeile 2295

**Aktueller Code:**
```cpp
if (skillActive && currentSkill == 20) {
    // A. Sicherheit: Manueller Abbruch NUR durch echte Joystick-Bewegung
    if (abs(joystickSpeed) > 100 || abs(joystickSteer) > 100) {
        // ...
    }
    // ...
}
```

**Implementierung:**
```cpp
if (skillActive && currentSkill == 20) {
    // *** NEU: Harte Abbruchbedingung für E-Stop ***
    if (skill8SafetyActive) {
        skillActive = false;
        currentSkill = 0;
        global_cmdCode = 0;
        missionTriggeredOnce = false;
        missionRequestSent = false;
        resetAll();  // Stop all motors
        if (ENABLE_DEBUG_SERIAL) debugPrintln(">> MISSION ABORT: E-Stop triggered!");
    }
    // ... restliche Logik
}
```

**Status:** ⬜ Geplant

---

### 🏗️ PHASE 2: Architektur-Fixes & Variablen-Hygiene

---

#### Task 2.1: Refactoring der `Send()` Funktion (Struktur erhalten!)

**Problem:** `Send()` überschreibt Parameter intern mit Globals wenn `global_cmdCode == 2`

**Datei:** `src/main.cpp` - `Send()` ca. Zeile 1373

**Aktueller Code:**
```cpp
void Send(int16_t uSteer, int16_t uSpeed) {
    // ...
    Command.start     = 0xABCD;
    Command.cmdCode   = (int16_t)global_cmdCode; 
    
    if (global_cmdCode == 2) {
        // MISSION: Sende die neu gesetzten Distanzen aus Skill 20
        Command.steer = lastEspNowSteer; 
        Command.speed = lastEspNowSpeed; 
    } else {
        // JOYSTICK: Sende normale Fahrwerte
        Command.steer = uSteer;
        Command.speed = uSpeed;
    }
    // ...
}
```

**Implementierung:**

1. **Lösche Forward-Declaration** (ca. Zeile 647 - prüfen):
   ```cpp
   void Send(int16_t uSteer, int16_t uSpeed);  // <- LÖSCHEN (doppelt)
   ```

2. **Bereinige Send() Funktion:**
   ```cpp
   void Send(int16_t uSteer, int16_t uSpeed) {
       static unsigned long lastHeartbeatSend = 0;
       if (millis() - lastHeartbeatSend < 50) return; 
       lastHeartbeatSend = millis();

       if (isHighVoltageShutdown || skill8SafetyActive) {
           uSteer = 0; uSpeed = 0;
       }

       Command.start     = 0xABCD;
       Command.cmdCode   = (int16_t)global_cmdCode; 
       
       // *** GEÄNDERT: Stumpf die Parameter verwenden ***
       Command.steer = uSteer;
       Command.speed = uSpeed;

       Command.maxSpeedL = global_maxSpeedL;
       Command.maxSpeedR = global_maxSpeedR;
       Command.accPct    = (int16_t)global_accel; 
       Command.brkPct    = (int16_t)global_brake;

       Command.checksum = // ...
       // ...
   }
   ```

3. **Passe `controlLogicTask` an:**
   ```cpp
   // In controlLogicTask, MISSIONS-START Block:
   if (!hoverboardIsBusy && !missionTriggeredOnce && !missionRequestSent) {
       global_cmdCode = 2;
       // *** GEÄNDERT: Parameter explizit übergeben ***
       Send(lastEspNowSteer, lastEspNowSpeed); 
       missionRequestSent = true; 
       shouldSend = false;
   }
   ```

**Status:** ⬜ Geplant

---

#### Task 2.2: Variable Shadowing & Redundanz auflösen

**Problem:** Variablen wie `global_maxSpeedL`, `lastEspNowSpeed`, `joystickSpeed` überschneiden sich

**Datei:** `src/main.cpp` - Globale Variablenbereich

**Analyse aktueller Variablen:**
```cpp
// Bereits vorhandene Trennung (ca. Zeile 546):
volatile int16_t joystickSpeed = 0; 
volatile int16_t joystickSteer = 0; 
volatile int16_t lastEspNowSpeed = 0; 
volatile int16_t lastEspNowSteer = 0; 
```

**Implementierung:**

1. **Konsolidiere Naming-Convention am Dateianfang:**
   ```cpp
   // Input-Quellen klar trennen:
   volatile int16_t input_JoySpeed = 0;      // war: joystickSpeed
   volatile int16_t input_JoySteer = 0;      // war: joystickSteer
   volatile int16_t input_EspNowSpeed = 0;   // war: lastEspNowSpeed
   volatile int16_t input_EspNowSteer = 0;   // war: lastEspNowSteer
   
   // Mission-Daten:
   volatile int16_t mission_DistL = 0;       // war: lastEspNowSteer (bei cmdCode==2)
   volatile int16_t mission_DistR = 0;       // war: lastEspNowSpeed (bei cmdCode==2)
   ```

2. **Ersetze alle Vorkommen** systematisch durch `StrReplaceFile`

**Status:** ⬜ Geplant (AUFWENDIG - viele Ersetzungen)

---

#### Task 2.3: Typen-Konflikte (Voltage Scaling)

**Problem:** Float/Integer Konvertierungen bei Spannungsberechnungen

**Datei:** `src/main.cpp` - `statusTask()` und `handleSystemStatus()`

**Aktueller Code in statusTask() (ca. Zeile 3200-3221):**
```cpp
uint32_t raw_pin_mv = analogReadMilliVolts(BLADE_BATTERY_PIN);
float calculated_mv = (float)raw_pin_mv * currentSettings.bladeBatteryFactor;
// ...
global_blade_voltage_mv = (int32_t)smoothed_blade_mv;  // <- Bereits explizit gecastet
```

**Analyse:** Der Code castet bereits explizit. Prüfen ob Vergleiche in `handleSystemStatus()` sauber sind.

**Implementierung (falls nötig):**
```cpp
// In handleSystemStatus() - sicherstellen dass Vergleiche in mV erfolgen:
int32_t bladeVoltage = global_blade_voltage_mv;  // bereits mV
int32_t threshold = (int32_t)(currentSettings.bladeMinShutdownVoltage * 1000); // V -> mV

if (bladeVoltage < threshold) {
    // ...
}
```

**Status:** ⬜ Zu prüfen ob Änderung nötig

---

### 🌐 PHASE 3: Web-UI & Speicher-Sicherheit

---

#### Task 3.2: EEPROM Initialisierung härten

**Problem:** Bei fehlendem CRC-Check werden Struct-Werte nicht zurückgesetzt

**Datei:** `src/main.cpp` - `loadSettings()`

**Implementierung:**
```cpp
void loadSettings() {
    Settings tempSettings;
    EEPROM.get(SETTINGS_ADDR, tempSettings);
    
    // CRC-Validierung
    uint32_t crc = calculateCRC32((uint8_t*)&tempSettings, sizeof(Settings) - sizeof(uint32_t));
    
    if (tempSettings.magic == 0xDEADBEEF && crc == /* gespeicherter CRC */) {
        currentSettings = tempSettings;
    } else {
        // *** NEU: Struct zurücksetzen vor Default-Zuweisung ***
        memset(&currentSettings, 0, sizeof(Settings));
        
        // Default-Werte setzen
        currentSettings.magic = 0xDEADBEEF;
        currentSettings.driveMinShutdownVoltage = 310;  // 31.0V
        // ... weitere Defaults
        saveSettings();
    }
}
```

**Status:** ⬜ Geplant

---

#### Task 3.3: Web-Command ACK (Missing Feedback)

**Problem:** Web-UI bekommt keine Bestätigung bei Mutex-Blockaden

**Datei:** `src/main.cpp` - `onWsEvent()`

**Implementierung:**
```cpp
// In onWsEvent(), nach Befehlsverarbeitung:
if (strcmp(msgType, "command") == 0) {
    JsonObject dataObj = doc["data"];
    int code = dataObj["code"];
    int value = dataObj["value"];
    
    // Befehl ausführen
    bool success = triggerSkill(code, value, 0, 0, 0);  // oder ähnlich
    
    // *** NEU: ACK senden ***
    if (success) {
        client->text("CMD: Skill " + String(code) + " [OK]");
    } else {
        client->text("CMD: Skill " + String(code) + " [FAILED]");
    }
}
```

**Status:** ⬜ Geplant

---

### 🧹 PHASE 4: Code-Hygiene

---

#### Task 4.1: Doppelte Hardware-Deklarationen entfernen

**Problem:** `pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP)` steht zweimal in `setup()`

**Datei:** `src/main.cpp` - `setup()`

**Implementierung:**
```cpp
// Suchen nach:
pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP);
pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP);  // <- LÖSCHEN
```

**Status:** ⬜ Geplant

---

#### Task 4.2: Template Overloads für Serial Debugging

**Problem:** Fehlende `float` Overloads für `debugPrint`/`debugPrintln`

**Datei:** `src/main.cpp` - nach Zeile 344

**Aktueller Code:**
```cpp
template <typename T>
void debugPrint(T msg) { ... }

template <typename T>
void debugPrintln(T msg) { ... }
```

**Implementierung:**
```cpp
// *** NEU: Float-Overloads hinzufügen ***
void debugPrint(float msg) {
    Serial.print(msg, 2); // 2 Nachkommastellen
    if (webLogActive) {
        webLogBuffer += String(msg, 2);
        if (webLogBuffer.length() > 2000) webLogBuffer = "--- BUFFER OVERFLOW ---\n";
    }
}

void debugPrintln(float msg) {
    Serial.println(msg, 2);
    if (webLogActive) {
        webLogBuffer += String(msg, 2) + "\n";
        if (webLogBuffer.length() > 2000) webLogBuffer = "--- BUFFER OVERFLOW ---\n";
    }
}
```

**Status:** ⬜ Geplant

---

#### Task 4.3: Refactoring des "Spaghetti-Monsters" (`triggerSkill`)

**Problem:** Riesiger `switch-case` in `triggerSkill()`

**Datei:** `src/main.cpp` - `triggerSkill()`

**Implementierung:**
```cpp
// Neue Hilfsfunktionen:
void handleDriveSkills(int skill, int p1, int p2, int16_t &steer, int16_t &speed);
void handleBladeSkills(int skill, int p1);
void handleSetupSkills(int skill, int p1, int p2, int p3);

void triggerSkill(...) {
    switch (skill) {
        case 1: case 2: case 3: case 4:  // Drive Skills
            handleDriveSkills(skill, p1, p2, steer, speed);
            break;
        case 5: case 6: case 10: case 11:  // Blade Skills
            handleBladeSkills(skill, p1);
            break;
        case 7: case 8: case 9: case 12: case 13: case 14: case 15: case 16: case 17: case 18: case 19: case 20:
            handleSetupSkills(skill, p1, p2, p3);
            break;
        // ...
    }
}
```

**Status:** ⬜ Geplant

---

## Test-Plan

Nach jeder Phase:
1. `pio run -e esp32s3` → Build erfolgreich?
2. Upload auf Test-ESP32
3. Manuelle Tests:
   - E-Stop während Mission
   - WebSocket Befehle
   - Spannungsgrenzwerte

---

## Änderungs-Log (wird während Implementierung gefüllt)

| Datum | Task | Geändert von | Beschreibung | Status |
|-------|------|--------------|--------------|--------|
| | | | | |

---

## Genehmigung

**Dieser Plan wartet auf Genehmigung.**

Bitte prüfen und freigeben für Implementierung.
