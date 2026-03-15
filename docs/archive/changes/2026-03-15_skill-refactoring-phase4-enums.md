# 🔧 SKILL REFACTORING - Phase 4: Enums für Skill-Codes

> **Datum:** 2026-03-15 00:45 CET  
> **Agent:** Kimi Code CLI  
> **Status:** 🔄 In Arbeit

---

## 🎯 ZIELSETZUNG

Hartcodierte Skill-Nummern (1, 2, 3, ...) durch benannte Enum-Werte ersetzen.

### Warum?
- `triggerSkill(6, 1)` → Was ist 6? Was ist 1?
- `triggerSkill(SKILL_BLADE_CONTROL, BLADE_ON)` → Selbsterklärend!

---

## 📋 SKILL-MAPPING

| Code | Name | Enum-Wert |
|------|------|-----------|
| 0 | (Reserved) | `SKILL_NONE` |
| 1 | System Reset | `SKILL_RESET` |
| 2 | 90° Turn | `SKILL_TURN_90` |
| 3 | Hold the Line | `SKILL_HOLD_LINE` |
| 4 | 180° Turn | `SKILL_TURN_180` |
| 5 | Blade Lift | `SKILL_BLADE_LIFT` |
| 6 | Blade Control | `SKILL_BLADE_CONTROL` |
| 7 | Set Limits | `SKILL_SET_LIMITS` |
| 8 | E-Stop/Capsize | `SKILL_ESTOP` |
| 9 | Reboot ESP32 | `SKILL_REBOOT` |
| 10 | Set Blade Speeds | `SKILL_SET_BLADE_SPEEDS` |
| 11 | Cable Reset | `SKILL_CABLE_RESET` |
| 12 | Drive System Levels | `SKILL_DRIVE_LEVELS` |
| 13 | Blade System Levels | `SKILL_BLADE_LEVELS` |
| 14 | Capsize Parameters | `SKILL_CAPSIZE_PARAMS` |
| 15 | PD Gains | `SKILL_PD_GAINS` |
| 16 | Blade Battery Factor | `SKILL_BLADE_BATTERY_FACTOR` |
| 17 | Drive Battery Factor | `SKILL_DRIVE_BATTERY_FACTOR` |
| 18 | Drive Unit Power | `SKILL_DRIVE_POWER` |
| 19 | Hoverboard Parameters | `SKILL_HOVERBOARD_PARAMS` |
| 20 | Distance Mission | `SKILL_DISTANCE_MISSION` |
| 255 | Heartbeat | `SKILL_HEARTBEAT` |

### Blade Control Sub-Codes
| Code | Bedeutung | Enum-Wert |
|------|-----------|-----------|
| -1 | Blade OFF | `BLADE_OFF_CMD` |
| 1 | Blade ON | `BLADE_ON_CMD` |
| 21000-22000 | Web PWM | (Bereich) |
| 25600 / 2 | Speed 1 | `KEY_BLADE_SPEED1` / `_ALT` |
| 26500 / 3 | Speed 2 | `KEY_BLADE_SPEED2` / `_ALT` |

---

## 📁 GEPLANTE ÄNDERUNGEN

| Datei | Zeile(n) | Änderung |
|-------|----------|----------|
| `main.cpp` | 176-210 | Enum-Definition hinzufügen |
| `main.cpp` | Alle `case X:` | `case SKILL_*:` |
| `main.cpp` | Alle `triggerSkill(X,` | `triggerSkill(SKILL_*,` |

---

## 📝 VERIFIKATION

- [x] Build erfolgreich (`pio run` - SUCCESS)
- [x] Keine Funktionsänderung
- [x] Code lesbarer

**Build-Datum:** 2026-03-15 00:48 CET
**Speicher:** RAM 15.9%, Flash 33.4%

---

## ✅ ZUSAMMENFASSUNG

| Code | Enum-Name | Status |
|------|-----------|--------|
| 0 | `SKILL_NONE` | ✅ Ersetzt |
| 1 | `SKILL_RESET` | ✅ Ersetzt |
| 2 | `SKILL_TURN_90` | ✅ Ersetzt |
| 3 | `SKILL_HOLD_LINE` | ✅ Ersetzt |
| 4 | `SKILL_TURN_180` | ✅ Ersetzt |
| 5 | `SKILL_BLADE_LIFT` | ✅ Ersetzt |
| 6 | `SKILL_BLADE_CONTROL` | ✅ Ersetzt |
| 7 | `SKILL_SET_LIMITS` | ✅ Ersetzt |
| 8 | `SKILL_ESTOP` | ✅ Ersetzt |
| 9 | `SKILL_REBOOT` | ✅ Ersetzt |
| 10 | `SKILL_SET_BLADE_SPEEDS` | ✅ Ersetzt |
| 11 | `SKILL_CABLE_RESET` | ✅ Ersetzt |
| 12 | `SKILL_DRIVE_LEVELS` | ✅ Ersetzt |
| 13 | `SKILL_BLADE_LEVELS` | ✅ Ersetzt |
| 14 | `SKILL_CAPSIZE_PARAMS` | ✅ Ersetzt |
| 15 | `SKILL_PD_GAINS` | ✅ Ersetzt |
| 16 | `SKILL_BLADE_BATTERY_FACTOR` | ✅ Ersetzt |
| 17 | `SKILL_DRIVE_BATTERY_FACTOR` | ✅ Ersetzt |
| 18 | `SKILL_DRIVE_POWER` | ✅ Ersetzt |
| 19 | `SKILL_HOVERBOARD_PARAMS` | ✅ Ersetzt |
| 20 | `SKILL_DISTANCE_MISSION` | ✅ Ersetzt |
| 255 | `SKILL_HEARTBEAT` | ✅ Definiert |

**Alle 21 Skill-Codes als Enum definiert!**

---

## 🔗 VERKNÜPFUNGEN

- Phase 1-2: `docs/archive/changes/2026-03-15_skill-refactoring-summary.md`
- Roadmap: `docs/archive/plans/skill-refactoring-roadmap.md`
