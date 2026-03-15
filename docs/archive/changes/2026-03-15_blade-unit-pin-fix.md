# 🚨 KRITISCHER BUG FIX: BLADE_UNIT_PIN

> **Datum:** 2026-03-15 00:42 CET  
> **Priorität:** 🔴 KRITISCH  
> **Status:** 🔄 In Arbeit

---

## 🎯 PROBLEMBESCHREIBUNG

### Der Bug
In `src/main.cpp` Zeilen 137-138 sind beide Power-Control-Pins auf **GPIO 20** definiert:

```cpp
// FALSCH (aktuell):
#define BLADE_UNIT_PIN               20        // Power Pin für Blade Unit
#define DRIVE_UNIT_PIN               20        // Power Pin für Drive Unit
```

### Warum ist das ein Problem?
1. **Hardware-Konflikt:** Beide Relais werden über denselben GPIO gesteuert
2. **Keine unabhängige Steuerung:** Blade und Drive können nicht getrennt ein/aus geschaltet werden
3. **Physische Pins:** Am PCB sind die Relais auf verschiedene GPIOs (19 und 20) verdrahtet

### Beweis aus dem Code
Die Kommentare in `setup()` bestätigen die beabsichtigte Zuordnung:

| Zeile | Kommentar | Implizierter GPIO |
|-------|-----------|-------------------|
| 1540 | `"Pin 19 auf LOW beim Start"` | BLADE_UNIT_PIN = 19 |
| 1543 | `"Pin 20 auf LOW beim Start"` | DRIVE_UNIT_PIN = 20 |
| 945 | `"Skill 6: Blade Unit ON (Pin 19 HIGH)"` | Debug-Log bestätigt 19 |

---

## 🔧 DIE LÖSUNG

```cpp
// KORREKTUR:
#define BLADE_UNIT_PIN               19        // GPIO 19 für Blade Unit
#define DRIVE_UNIT_PIN               20        // GPIO 20 für Drive Unit (unverändert)
```

---

## ⚠️ WICHTIGE HINWEISE

### Vor dem Firmware-Update:
1. **Hardware-Test empfohlen:** Blade-Relais und Drive-Relais einzeln testen
2. **Sicherheit:** Messer sollte bei Test **abgenommen** sein
3. **Verifikation:** Mit Multimeter prüfen, ob GPIO 19 und 20 korrekt verdrahtet sind

### Nach dem Firmware-Update:
1. Skill 6 testen (Blade ON/OFF) - sollte nur GPIO 19 beeinflussen
2. Skill 18 testen (Drive Power ON/OFF) - sollte nur GPIO 20 beeinflussen
3. Beide gleichzeitig testen - sollten unabhängig funktionieren

---

## 📝 VERIFIKATION

- [x] Build erfolgreich (`pio run` - SUCCESS)
- [x] Code-Review
- [ ] Hardware-Test (empfohlen vor Einsatz!)

**Build-Datum:** 2026-03-15 00:42 CET
**Speicher:** RAM 15.9%, Flash 33.4%

---

## 🔗 VERKNÜPFUNGEN

- Pinout-Doku: `AGENTS.md` (GPIO Tabelle)
- Tech-Manual: `docs/TECHNICAL_MANUAL.md` (Section 1.4)
- Refactoring: `docs/archive/changes/2026-03-15_skill-refactoring-summary.md`
