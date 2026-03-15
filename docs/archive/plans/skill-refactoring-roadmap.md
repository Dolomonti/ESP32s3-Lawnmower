# 🗺️ SKILL SYSTEM REFACTORING - ROADMAP

> **Erstellt:** 2026-03-15 00:35 CET  
> **Status:** Phase 1 ✅ Abgeschlossen | Phase 2-4 ⏳ Geplant  
> **Ziel:** Code-Qualität von "funktional" auf "professionell" heben

---

## 📊 AUSGANGSSITUATION (Nach Phase 1)

### ✅ Bereits erledigt (Phase 1)
| Problem | Status |
|---------|--------|
| Duplizierte Winkel-Normalisierung | ✅ Behoben |
| Lambda-Overkill | ✅ Behoben |
| Monster-Zeilen (>200 Zeichen) | ✅ Behoben |
| Code-Formatierung | ✅ Verbessert |

### ⏳ Noch offen (31 - 4 = 27 Probleme)
| Kategorie | Anzahl | Priorität |
|-----------|--------|-----------|
| **Magic Numbers** | 12 | 🔴 Hoch |
| **Architektur** | 8 | 🟡 Mittel |
| **Validierung** | 4 | 🟡 Mittel |
| **Dokumentation** | 3 | 🟢 Niedrig |

---

## 🎯 PHASENPLAN

```
┌─────────────────────────────────────────────────────────────────┐
│  PHASE 1: ✅ Kritische Probleme (ERLEDIGT)                      │
│  └── Duplikate, Lambda, Formatierung                            │
├─────────────────────────────────────────────────────────────────┤
│  PHASE 2: 🔴 Magic Numbers & Konstanten (EMPFOHLEN)            │
│  └── Hartcodierte Werte durch Konstanten ersetzen               │
├─────────────────────────────────────────────────────────────────┤
│  PHASE 3: 🟡 Fehlerbehandlung & Validierung                    │
│  └── EEPROM-Fehlercheck, PWM-Validierung, Bounds-Checking       │
├─────────────────────────────────────────────────────────────────┤
│  PHASE 4: 🟢 Architektur-Verbesserungen                        │
│  └── Enums, Helper-Funktionen, Code-Struktur                    │
└─────────────────────────────────────────────────────────────────┘
```

---

## 🔴 PHASE 2: Magic Numbers & Konstanten

### Warum ist das wichtig?
**Magic Numbers** sind hartcodierte Werte ohne erklärenden Namen. Sie machen den Code:
- Schwer verständlich (Was bedeutet 25600?)
- Fehleranfällig (Tippfehler werden nicht erkannt)
- Schwer wartbar (Änderung an 10 Stellen nötig)

### Konkrete Probleme

| # | Zeile | Magic Number | Bedeutung | Konstante |
|---|-------|--------------|-----------|-----------|
| 1 | 818 | `90.0f` | 90° Drehung | `TURN_ANGLE_90` |
| 2 | 820 | `180.0f` | 180° Drehung | `TURN_ANGLE_180` |
| 3 | 887 | `> 0` / `< 0` | Richtung | `DIRECTION_LEFT/RIGHT` |
| 4 | 905 | `20000` | PWM-Offset | `PWM_WEB_OFFSET` |
| 5 | 905 | `21000-22000` | PWM-Range | `PWM_WEB_MIN/MAX` |
| 6 | 909 | `25600` | Speed 1 Key | `KEY_BLADE_SPEED1` |
| 7 | 912 | `26500` | Speed 2 Key | `KEY_BLADE_SPEED2` |
| 8 | 909 | `2` | Speed 1 Alt | `KEY_BLADE_SPEED1_ALT` |
| 9 | 912 | `3` | Speed 2 Alt | `KEY_BLADE_SPEED2_ALT` |
| 10 | 1001 | `1000` | Config Mode | `CONFIG_ANGLE_MODE` |
| 11 | 1004 | `1001` | Config Mode | `CONFIG_TIMEOUT_MODE` |
| 12 | 984 | `1000` | Min RPM | `MIN_RPM_THRESHOLD` |

### Beispiel-Refactoring

**VORHER (unklar):**
```cpp
else if (key == 25600 || key == 2) {
    // Was ist 25600? Warum 2?
    current_blade_pwm = currentSettings.bladeWorkingSpeed;
}
```

**NACHHER (selbsterklärend):**
```cpp
else if (key == KEY_BLADE_SPEED1 || key == KEY_BLADE_SPEED1_ALT) {
    current_blade_pwm = currentSettings.bladeWorkingSpeed;
}
```

### Aufwand
- **Zeit:** ~30 Minuten
- **Risiko:** ⭐ Niedrig (nur Namen ändern, Werte bleiben)
- **Build-Test:** Erforderlich

---

## 🟡 PHASE 3: Fehlerbehandlung & Validierung

### 3.1 EEPROM-Fehlerbehandlung (`saveSettings`)

**Problem:** `saveSettings()` gibt void zurück - kein Feedback ob Speichern geklappt hat.

**Lösung:**
```cpp
// VORHER:
saveSettings();  // Hoffentlich geklappt?

// NACHHER:
if (!saveSettings()) {
    DEBUG_LOG("ERROR: Failed to save settings to EEPROM!");
    // Optional: Retry oder Fehler an UI melden
}
```

### 3.2 PWM-Wert-Validierung (Skill 6)

**Problem:** PWM-Werte werden ohne Prüfung übernommen.

**Lösung:**
```cpp
// NACHHER:
int16_t pwmValue = key - PWM_WEB_OFFSET;
if (pwmValue < BLADE_PWM_MIN || pwmValue > BLADE_PWM_MAX) {
    DEBUG_PRINTF("ERROR: PWM value %d out of range!\n", pwmValue);
    return;
}
current_blade_pwm = pwmValue;
```

### 3.3 Skill-Nummer Bounds-Checking

**Problem:** Jede Zahl 0-255 wird als Skill akzeptiert.

**Lösung:**
```cpp
// NACHHER:
if (skill > MAX_SKILL_NUMBER) {
    DEBUG_PRINTF("ERROR: Invalid skill number %d\n", skill);
    return;
}
```

### Aufwand
- **Zeit:** ~45 Minuten
- **Risiko:** ⭐⭐ Mittel (Logik-Änderungen)
- **Test:** Erforderlich (Hardware oder Simulation)

---

## 🟢 PHASE 4: Architektur-Verbesserungen

### 4.1 Enum für Skill-Nummern

**Problem:** Hartcodierte Zahlen (1, 2, 3, ...) überall im Code.

**Lösung:**
```cpp
enum SkillCode {
    SKILL_RESET = 1,
    SKILL_TURN_90 = 2,
    SKILL_HOLD_LINE = 3,
    SKILL_TURN_180 = 4,
    SKILL_BLADE_LIFT = 5,
    SKILL_BLADE_CONTROL = 6,
    SKILL_SET_LIMITS = 7,
    SKILL_ESTOP = 8,
    SKILL_REBOOT = 9,
    SKILL_SET_BLADE_SPEEDS = 10,
    SKILL_CABLE_RESET = 11,
    // ... etc
};
```

### 4.2 Helper-Funktion: `stopBladeSafely()`

**Problem:** Safety-Code ist 4x dupliziert (in Phase 1 nur formatiert, nicht extrahiert).

**Lösung:**
```cpp
inline void stopBladeSafely(const char* reason) {
    currentBladeState = BLADE_OFF;
    digitalWrite(BLADE_UNIT_PIN, LOW);
    current_blade_pwm = BLADE_ZERO_US;
    DEBUG_PRINTF("Blade stopped: %s\n", reason);
}
```

### 4.3 BLADE_UNIT_PIN Bug fixen

**KRITISCH:** In Zeile 137-138 sind beide Pins auf 20 definiert!

```cpp
// VORHER (BUG):
#define BLADE_UNIT_PIN 20  // Sollte 19 sein!
#define DRIVE_UNIT_PIN 20

// NACHHER (FIXED):
#define BLADE_UNIT_PIN 19  // GPIO 19 für Blade Relay
#define DRIVE_UNIT_PIN 20  // GPIO 20 für Drive Relay
```

⚠️ **ACHTUNG:** Dies erfordert Hardware-Test!

### Aufwand
- **Zeit:** ~60 Minuten
- **Risiko:** ⭐⭐⭐ Hoch (BLADE_UNIT_PIN Bug)
- **Test:** Hardware-Test EMPFOHLEN

---

## 📋 EMPFEHLUNG: Reihenfolge

### Option A: Konservativ (Empfohlen)
1. ✅ **Phase 1** - DONE
2. 🔴 **Phase 2** - Magic Numbers (sicher, keine Logik-Änderung)
3. 🟢 **Phase 4** - Nur Enums (sicher)
4. ⏸️ **Phase 3** & BLADE_UNIT_PIN - Später mit Hardware-Test

### Option B: Alles auf einmal
⚠️ Riskant - viele Änderungen auf einmal

### Option C: Nur Kritisches
Nur Phase 4.3 (BLADE_UNIT_PIN Bug) fixen - Hardware-Fehler!

---

## 🎯 ZUSAMMENFASSUNG

| Phase | Dauer | Risiko | Empfohlen |
|-------|-------|--------|-----------|
| 2 - Magic Numbers | 30 min | ⭐ Niedrig | ✅ Sofort |
| 3 - Validierung | 45 min | ⭐⭐ Mittel | 🟡 Nach Test |
| 4 - Architektur | 60 min | ⭐⭐⭐ Hoch | 🟡 Nach Test |

**Meine Empfehlung:** Phase 2 jetzt durchführen (sicher, schnell, großer Nutzen). Phase 3 & 4 nach Hardware-Test.

---

## 🔗 VERKNÜPFUNGEN

- Aktueller Stand: `docs/archive/changes/2026-03-15_skill-refactoring-summary.md`
- Ursprüngliches Audit: `docs/archive/changes/2026-03-15_skill-code-audit.md`
