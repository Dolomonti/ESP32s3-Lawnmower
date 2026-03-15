# 🔍 CODE AUDIT: 31 Veraltete Kodierungsprobleme im Skill-System

> **Datum:** 2026-03-15  
> **Datei:** `src/main.cpp`  
> **Bereich:** Skill-Handler (Zeilen 794-1006)

---

## 📊 ZUSAMMENFASSUNG

| Kategorie | Anzahl | Probleme |
|-----------|--------|----------|
| Magic Numbers | 12 | Hartcodierte Werte ohne Konstanten |
| Code Duplication | 5 | Wiederholte Code-Blöcke |
| Format/Stil | 6 | Lange Zeilen, mehrere Zuweisungen |
| Architektur | 5 | Lambda, Fehlerbehandlung, Parameter |
| Validierung | 3 | Fehlende Bereichsprüfungen |
| **GESAMT** | **31** | |

---

## 🔴 KRITISCHE PROBLEME (Priorität: Hoch)

### 1. DUPLIZIERTER CODE: Winkel-Normalisierung
**Ort:** `handleDriveSkills()`, Skill 2 (Zeilen 820-821) und Skill 4 (Zeilen 842-843)

```cpp
// Skill 2:
if (targetAngle > 180) targetAngle -= 360;
if (targetAngle < -180) targetAngle += 360;

// Skill 4: IDENTISCHER CODE
if (targetAngle > 180) targetAngle -= 360;
if (targetAngle < -180) targetAngle += 360;
```

**Soll:** `normalizeAngle(targetAngle)` verwenden (bereits in Zeile 2701 definiert!)

---

### 2. LAMBDA-OVERKILL: bladeAllowed()
**Ort:** `handleBladeSkills()`, Skill 6 (Zeile 891)

```cpp
auto bladeAllowed = []() -> bool { return (currentBladeHeight == BLADE_HEIGHT_DOWN); };
```

**Problem:** Lambda-Funktion für einfache bool-Abfrage. 5x aufgerufen mit identischem Ergebnis.

**Soll:** Direkte Abfrage oder inline Konstante

---

### 3. DUPLIZIERTER SAFETY-CHECK
**Ort:** `handleBladeSkills()`, Skill 6 (Zeilen 897, 900, 903, 906)

```cpp
// 4x identischer Code-Block:
currentBladeState = BLADE_OFF; 
digitalWrite(BLADE_UNIT_PIN, LOW); 
current_blade_pwm = BLADE_ZERO_US;
```

---

## 🟡 MAGIC NUMBERS (12 Stück)

| # | Zeile | Wert | Bedeutung | Lösung |
|---|-------|------|-----------|--------|
| 4 | 818 | `90.0` | 90-Grad-Drehung | `constexpr float TURN_ANGLE_90 = 90.0f;` |
| 5 | 840 | `180.0` | 180-Grad-Drehung | `constexpr float TURN_ANGLE_180 = 180.0f;` |
| 6 | 891 | `> 0` / `< 0` | Richtung | `enum Direction { LEFT = -1, RIGHT = 1 };` |
| 7 | 898 | `20000` | PWM-Offset | `constexpr int PWM_WEB_OFFSET = 20000;` |
| 8 | 898 | `21000..22000` | PWM-Range | `constexpr int PWM_MIN_WEB = 21000;` etc. |
| 9 | 902 | `25600` | Speed 1 Key | `constexpr int KEY_SPEED1 = 25600;` |
| 10 | 905 | `26500` | Speed 2 Key | `constexpr int KEY_SPEED2 = 26500;` |
| 11 | 902 | `2` | Alternative Key | `constexpr int KEY_SPEED1_ALT = 2;` |
| 12 | 905 | `3` | Alternative Key | `constexpr int KEY_SPEED2_ALT = 3;` |
| 13 | 936 | `1000` | Config Mode | `constexpr int CONFIG_ANGLE = 1000;` |
| 14 | 937 | `1001` | Config Mode | `constexpr int CONFIG_TIMEOUT = 1001;` |
| 15 | 918 | `1000` | Min RPM | `constexpr int MIN_RPM_THRESHOLD = 1000;` |

---

## 🟠 FORMATIERUNGS-PROBLEME (6 Stück)

### 16. Mehrere Zuweisungen pro Zeile (Skill 10)
**Zeile 911:**
```cpp
currentSettings.bladeWorkingSpeed = param1; currentSettings.bladeStage2Speed = param2; currentSettings.bladeMaxSpeed = param3; currentSettings.bladeCableResetRpm = param4; saveSettings();
```
**Länge:** 212 Zeichen!

### 17. Mehrere Zuweisungen pro Zeile (Skill 12)
**Zeile 961:**
```cpp
currentSettings.driveMinShutdownVoltage = param1; currentSettings.driveSafetyModeVoltage = param2; ... saveSettings();
```
**Länge:** ~280 Zeichen!

### 18. Mehrere Zuweisungen pro Zeile (Skill 13)
**Zeile 965:**
```cpp
currentSettings.bladeMinShutdownVoltage = param1; currentSettings.bladeSafetyModeVoltage = param2; ... saveSettings();
```

### 19-20. Mehrere Zuweisungen (Skill 2, 3)
**Zeile 815:** `resetAll(); resetHorizon();`
**Zeile 831:** `resetAll(); holdLine = true; skillActive = true; ...`

### 21. Inkonsistente Formatierung
Manchmal `if (a) { b; }` auf einer Zeile, manchmal ausgeweitet.

---

## 🔵 ARCHITEKTUR-PROBLEME (5 Stück)

### 22. MULTIPLE saveSettings() AUFRUFE
**Ort:** Skill 8 (Zeilen 936, 937, 969)

```cpp
if (param1 == 1000) { ... saveSettings(); return; }
if (param1 == 1001) { ... saveSettings(); return; }
```

**Problem:** 3x saveSettings() im selben Skill statt einmal am Ende.

---

### 23. INKONSISTENTE CASTING-FAKTOREN
**Ort:** Skill 15, 16, 17

```cpp
// Skill 15: / 100.0f
if (param1 > 0) currentSettings.Kp = (float)param1 / 100.0f;

// Skill 16: / 100.0 (ohne f)
currentSettings.bladeBatteryFactor = (float)param1 / 100.0;

// Skill 17: / 1000.0
currentSettings.driveBatteryFactor = (float)param1 / 1000.0;
```

**Problem:** Inkonsistente Faktoren (100, 1000) ohne Dokumentation.

---

### 24. UNBENUTZTE PARAMETER
**Ort:** `handleDriveSkills()`

```cpp
void handleDriveSkills(uint8_t skill, int16_t param1, int16_t param2, int16_t param3, 
                       int16_t param4, int16_t param5, int16_t param6)
```

**Problem:** param2-param6 werden in Skills 1-4 ignoriert.

---

### 25. HARTCODIERTE SKILL-NUMMERN
**Ort:** `triggerSkill()` (Zeile 998-1005)

```cpp
if (skill <= 4 || skill == 20) { ... }
else if (skill == 5 || skill == 6 || skill == 10 || skill == 11) { ... }
```

**Problem:** Magic Numbers statt enum oder konstanter Arrays.

---

### 26. KEINE BOUNDS-CHECKING
**Ort:** Alle Handler

**Problem:** Keine Überprüfung ob Skill-Nummer gültig ist (0-255 wird akzeptiert).

---

## 🟣 VALIDIERUNGS-PROBLEME (3 Stück)

### 27. KEINE PARAMETER-VALIDIERUNG (Skill 7)
**Zeile 930-931:**

```cpp
if (param3 > 0) currentSettings.currentMaxSpeed = constrain(param3, 0, MAX_SPEED);
if (param2 > 0) currentSettings.currentMaxSteer = constrain(param2, 0, MAX_STEER);
```

**Problem:** Nur auf > 0 geprüft, nicht auf vernünftige Maximalwerte.

---

### 28. KEINE FEHLERBEHANDLUNG FÜR saveSettings()
**Ort:** Überall

```cpp
saveSettings();  // Rückgabewert wird ignoriert
```

**Problem:** Wenn EEPROM-Schreiben fehlschlägt, gibt es keinen Hinweis.

---

### 29. KEINE PWM-WERT-VALIDIERUNG (Skill 6)
**Zeilen 899, 902, 905:**

```cpp
current_blade_pwm = key - 20000;  // Keine Prüfung ob im gültigen Bereich!
```

---

## ⚫ WEITERE PROBLEME (2 Stück)

### 30. INLINE WINKEL-NORMALISIERUNG
**Ort:** Skill 2, 4

```cpp
// Statt normalizeAngle() zu verwenden:
if (targetAngle > 180) targetAngle -= 360;
if (targetAngle < -180) targetAngle += 360;
```

**Hinweis:** `normalizeAngle()` ist bereits in Zeile 2701 definiert!

---

### 31. STATISCHE VARIABLE IN SKILL 8
**Zeile 942:**

```cpp
static unsigned long lastEstopPrint = 0;
```

**Problem:** Statische Variable in Handler-Funktion - bei mehreren Aufrufen geteilter Zustand.

---

## 📋 EMPFOHLENE REFACTORING-MAßNAHMEN

### Sofort (Kritisch):
1. `normalizeAngle()` in Skill 2 und 4 verwenden
2. Lambda `bladeAllowed()` entfernen
3. Safety-Check in Funktion extrahieren

### Kurzfristig (Magic Numbers):
4. Konstanten für alle Magic Numbers definieren
5. Enum für Skill-Nummern erstellen

### Mittelfristig (Architektur):
6. `saveSettings()` nur einmal pro Skill aufrufen
7. Einheitliche Parameter-Validierung
8. Fehlerbehandlung für EEPROM-Operationen

---

## 📝 BEISPIEL-REFACTORING (Skill 6)

**Vorher:**
```cpp
case 6: {
    auto bladeAllowed = []() -> bool { ... };
    if (key == -1) { ... }
    else if (key == 1) { if (bladeAllowed()) ... }
    // ... 4x duplizierter Safety-Code
}
```

**Nachher:**
```cpp
case SKILL_BLADE_CONTROL: {
    if (!isBladeAllowed()) {
        stopBladeSafely();
        break;
    }
    switch (key) {
        case BLADE_OFF: stopBlade(); break;
        case BLADE_ON:  startBlade(); break;
        // ...
    }
}
```

---

## 🔗 VERKNÜPFUNGEN

- Übergeordnet: `docs/archive/changes/2026-03-15_pinout-correction.md`
- Siehe auch: `docs/TECHNICAL_MANUAL.md` (Skill API Dokumentation)
