# To-Do 6: ESP32 Refactoring & Stabilitäts-Update

> **Erstellt:** 2026-03-15  
> **Status:** In Bearbeitung  
> **Ziel:** Architektur-Schwächen beheben, Heap-Fragmentierung vermeiden, Thread-Sicherheit gewährleisten

**⚠️ WICHTIG:** Diese Datei ist unser "Faden" - nach JEDEM Schritt aktualisieren!

---

## Phase 1: Speichermanagement & Haltbarkeit

### Phase 1.2: Umstellung von EEPROM auf Preferences (NVS) ✅ ABGESCHLOSSEN

**Problem:** EEPROM.h hat kein Wear-Leveling und zerstört den Flash-Speicher bei regelmäßigem Schreiben.

**Ziel:** Umstellung auf Preferences (NVS) mit Wear-Leveling.

#### Schritt 1.2.1: Include austauschen ✅ ERLEDIGT
- [x] `#include <EEPROM.h>` entfernt
- [x] `#include <Preferences.h>` hinzugefügt (Zeile 272)
- [x] **AKTUALISIERT:** 2026-03-15 05:00 CET - Include erfolgreich getauscht

#### Schritt 1.2.2: `saveSettings()` umschreiben ✅ ERLEDIGT
- [x] `Preferences prefs;` erstellt
- [x] `prefs.begin("mower", false);` öffnet NVS
- [x] `prefs.putBytes("settings", &currentSettings, sizeof(Settings));`
- [x] `prefs.putUInt("crc", calculated_crc);`
- [x] `prefs.end();` schließt NVS
- [x] Fehlerbehandlung für NVS-Öffnung hinzugefügt
- [x] **AKTUALISIERT:** 2026-03-15 05:02 CET - saveSettings() auf NVS umgestellt

#### Schritt 1.2.3: `loadSettings()` umschreiben ✅ ERLEDIGT
- [x] `Preferences prefs;` erstellt
- [x] `prefs.begin("mower", true);` read-only geöffnet
- [x] `prefs.getBytesLength("settings")` prüft ob Daten existieren
- [x] `prefs.getBytes("settings", &currentSettings, sizeof(Settings));`
- [x] `prefs.getUInt("crc", 0);` für CRC-Check
- [x] `prefs.end();` schließt NVS
- [x] **AKTUALISIERT:** 2026-03-15 05:05 CET - loadSettings() auf NVS umgestellt

#### Schritt 1.2.4: WiFi Credentials umschreiben ✅ ERLEDIGT
- [x] `writeWiFiCredentialsToEEPROM()` → `writeWiFiCredentialsToNVS()` umbenannt
- [x] `readWiFiCredentialsFromEEPROM()` → `readWiFiCredentialsFromNVS()` umbenannt
- [x] Beide Funktionen auf Preferences umgestellt
- [x] Alle 3 Aufrufe aktualisiert (Zeilen 1857, 2337, 2721)
- [x] **AKTUALISIERT:** 2026-03-15 05:08 CET - WiFi Credentials auf NVS umgestellt

#### Schritt 1.2.5: `eepromSaveTask` anpassen ✅ ERLEDIGT
- [x] Task umbenannt: `eepromSaveTask` → `nvsSaveTask`
- [x] Kommentar aktualisiert (NVS mit Wear-Leveling)
- [x] Debug-Logging auf "NVS" angepasst
- [x] Task-Erzeugung in setup() aktualisiert (Zeile 2139)
- [x] **AKTUALISIERT:** 2026-03-15 05:11 CET - Task auf NVS umgestellt

#### Schritt 1.2.6: Build testen ✅ ERLEDIGT
- [x] `pio run -e esp32s3` erfolgreich
- [x] EEPROM.begin() entfernt (war überflüssig)
- [x] **ERGEBNIS:** SUCCESS - RAM: 15.9%, Flash: 33.5%
- [x] **AKTUALISIERT:** 2026-03-15 05:15 CET - Phase 1.2 ABGESCHLOSSEN

---

### Phase 1.1: Beseitigung der Heap-Fragmentierung (String-Klasse) ✅ ABGESCHLOSSEN

**Problem:** Arduino String-Klasse fragmentiert den Heap bei `+=` Operationen.

#### Schritt 1.1.1: `webLogBuffer` umstellen ✅ ERLEDIGT
- [x] `String webLogBuffer` → `char webLogBuffer[2048]` (Ringpuffer)
- [x] `webLogWriteIndex`, `webLogReadIndex`, `webLogOverflow` hinzugefügt
- [x] `addToWebLogBuffer()` - sicheres Schreiben in Ringpuffer
- [x] `getFromWebLogBuffer()` - Auslesen für WebSocket
- [x] `webLogHasData()`, `clearWebLogBuffer()` Hilfsfunktionen
- [x] **AKTUALISIERT:** 2026-03-15 05:25 CET

#### Schritt 1.1.2: `logToWebpage()` umstellen ✅ ERLEDIGT
- [x] `logToWebpage(const String&)` → `logToWebpage(const char*)`
- [x] Alle 11 Aufrufe auf char-Buffer umgestellt (snprintf)
- [x] **AKTUALISIERT:** 2026-03-15 05:26 CET

#### Schritt 1.1.3: `debugPrint` Funktionen angepasst ✅ ERLEDIGT
- [x] Templates auf char-Buffer umgestellt
- [x] IPAddress Overloads angepasst
- [x] float Overloads angepasst
- [x] **AKTUALISIERT:** 2026-03-15 05:26 CET

#### Schritt 1.1.4: Build testen ✅ ERLEDIGT
- [x] `pio run -e esp32s3` erfolgreich
- [x] **ERGEBNIS:** SUCCESS - RAM: 16.5% (+0.6%), Flash: 33.5%
- [x] **AKTUALISIERT:** 2026-03-15 05:27 CET - Phase 1.1 ABGESCHLOSSEN

---

## Phase 2: FreeRTOS Thread-Sicherheit ✅ ABGESCHLOSSEN

**Problem:** Globale Variablen werden von mehreren Tasks ohne Schutz verwendet.

#### Schritt 2.1: Include und atomare Typen einführen ✅ ERLEDIGT
- [x] `#include <atomic>` hinzugefügt (Zeile 275)
- [x] **AKTUALISIERT:** 2026-03-15 05:32 CET

#### Schritt 2.2: Globale atomare Variablen definiert ✅ ERLEDIGT
- [x] `skill8SafetyActive` → `std::atomic<bool>{false}` (Zeile 644)
- [x] `isInSafetyMode` → `std::atomic<bool>{false}` (Zeile 364)
- [x] `skillActive` → `std::atomic<bool>{false}` (Zeile 682)
- [x] `currentSkill` → `std::atomic<int>{0}` (Zeile 683)
- [x] `hoverboardIsBusy` → `std::atomic<bool>{false}` (Zeile 690)
- [x] `currentBladeState` war schon Enum (nicht geändert)
- [x] **AKTUALISIERT:** 2026-03-15 05:32 CET

#### Schritt 2.3: JSON-Zugriffe mit .load() angepasst ✅ ERLEDIGT
- [x] `doc["estop"] = skill8SafetyActive.load()` (Zeile 3227)
- [x] `doc["turn_active"] = skillActive.load()` (Zeile 3302)
- [x] **AKTUALISIERT:** 2026-03-15 05:33 CET

#### Schritt 2.4: Build testen ✅ ERLEDIGT
- [x] `pio run -e esp32s3` erfolgreich
- [x] **ERGEBNIS:** SUCCESS - RAM: 16.5%, Flash: 33.5%
- [x] **AKTUALISIERT:** 2026-03-15 05:33 CET - Phase 2 ABGESCHLOSSEN

---

## Phase 3: Netzwerk-Flexibilität (DHCP) ✅ ABGESCHLOSSEN

**Problem:** Statische IP 192.168.1.123 funktioniert nicht in allen Netzwerken.

#### Schritt 3.1: DHCP als Standard ✅ ERLEDIGT
- [x] `WiFi.config()` entfernt (Zeilen 2711-2714 gelöscht)
- [x] DHCP-Modus aktiv (automatisch ohne statische Konfiguration)
- [x] Kommentar hinzugefügt für Dokumentation
- [x] **AKTUALISIERT:** 2026-03-15 05:38 CET

#### Schritt 3.2: Build testen ✅ ERLEDIGT
- [x] `pio run -e esp32s3` erfolgreich
- [x] **ERGEBNIS:** SUCCESS - RAM: 16.5%, Flash: 33.5% (-0.7% kleiner!)
- [x] **AKTUALISIERT:** 2026-03-15 05:39 CET - Phase 3 ABGESCHLOSSEN

---

## Phase 4: Architektur & Modularisierung ✅ ABGESCHLOSSEN

**Ziel:** main.cpp entlasten, "God Class" auflösen.

#### Schritt 4.1: Config.h erstellen ✅ ERLEDIGT
- [x] Datei `src/Config.h` erstellt (10.4 KB)
- [x] Alle `#define` Konstanten extrahiert (PINS, TIMING, TEMPERATURE)
- [x] Pin-Definitionen extrahiert (GPIO 1-46)
- [x] Settings-Struct verschoben (33 Felder)
- [x] Enum-Definitionen verschoben (SkillCode, BladeState, BladeHeight)
- [x] Extern-Deklarationen für globale Variablen hinzugefügt
- [x] **AKTUALISIERT:** 2026-03-15 05:45 CET

#### Schritt 4.2: Config.h in main.cpp einbinden ✅ ERLEDIGT
- [x] `#include "Config.h"` hinzugefügt
- [x] Doppelte Definitionen entfernt:
  - Alle #define PINS
  - Alle constexpr Konstanten
  - Settings-Struct
  - Alle Enums
  - WEB_LOG_BUFFER_SIZE Konflikt gelöst
- [x] Atomare Variablen korrekt definiert (nicht doppelt)
- [x] DEBUG_LOG Makros in main.cpp belassen (Template-Problematik)
- [x] Build: SUCCESS
- [x] **AKTUALISIERT:** 2026-03-15 05:55 CET

#### Schritt 4.3: HoverboardComm.h/cpp erstellen ✅ ERLEDIGT
- [x] `src/HoverboardComm.h` erstellt (Deklarationen)
- [x] `src/HoverboardComm.cpp` erstellt (Definitionen)
- [x] `SerialCommand` und `SerialFeedback` Structs verschoben
- [x] Alle globalen Variablen (Command, Feedback, etc.) kapseln
- [x] Mutexe (bufferMutex, feedbackMutex) kapseln
- [x] In main.cpp eingebunden:
  - `#include "HoverboardComm.h"`
  - Extern-Deklarationen für shared Variablen
  - HoverSerial als Pointer umgestellt
  - initHoverboardComm() Aufruf
- [x] Build: SUCCESS
- [x] **AKTUALISIERT:** 2026-03-15 06:15 CET

#### Schritt 4.4: SensorManager.h/cpp erstellen ✅ ERLEDIGT (Vereinfacht)
- [x] `src/SensorManager.h` erstellt
- [x] `src/SensorManager.cpp` erstellt
- [x] `readBladeBatteryVoltage()` - Spannungsmessung ausgelagert
- [x] `smoothVoltage()` - Glättungsfunktion ausgelagert
- [x] **HINWEIS:** mpuReadTask bleibt in main.cpp (zu viele Abhängigkeiten zu Skills)
- [x] In main.cpp eingebunden
- [x] Build: SUCCESS
- [x] **AKTUALISIERT:** 2026-03-15 06:25 CET

#### Schritt 4.5: NetworkManager.h/cpp erstellen ✅ ERLEDIGT (Vereinfacht)
- [x] `src/NetworkManager.h` erstellt
- [x] `src/NetworkManager.cpp` erstellt
- [x] `setupWiFiAP()` - AP-Setup mit DNS-Server ausgelagert
- [x] `connectToWiFiSTA()` - STA-Verbindung ausgelagert
- [x] `disconnectWiFi()`, `getLocalIPString()`, `isConnectedToHomeNetwork()`
- [x] In main.cpp eingebunden
- [x] Build: SUCCESS
- [x] **AKTUALISIERT:** 2026-03-15 06:35 CET

#### Schritt 4.6: Finale Build-Tests ✅ ERLEDIGT
- [x] `pio run -e esp32s3` erfolgreich
- [x] **ERGEBNIS:** SUCCESS - RAM: 16.5%, Flash: 33.5%
- [x] **AKTUALISIERT:** 2026-03-15 06:35 CET - Phase 4 ABGESCHLOSSEN

#### Schritt 4.6: Finale Build-Tests
- [ ] `pio run -e esp32s3` erfolgreich
- [ ] **ERGEBNIS:** _

---

## Phase 5: Hardware-Sicherheit (Not-Aus) ✅ ABGESCHLOSSEN

**Problem:** E-Stop ist nur Software-basiert.

#### Schritt 5.1: Warnkommentar hinzufügen ✅ ERLEDIGT
- [x] Großer Warnkommentar vor `case SKILL_ESTOP` eingefügt
- [x] Hardware-Verdrahtung dokumentiert (GPIO 15, 19, 20)
- [x] Sicherheitsstandards erwähnt (EN ISO 12100, EN 60204-1)
- [x] Empfohlene Schaltung beschrieben
- [x] **AKTUALISIERT:** 2026-03-15 05:42 CET

#### Schritt 5.2: Build testen ✅ ERLEDIGT
- [x] `pio run -e esp32s3` erfolgreich
- [x] **ERGEBNIS:** SUCCESS - RAM: 16.5%, Flash: 33.5%
- [x] **AKTUALISIERT:** 2026-03-15 05:43 CET - Phase 5 ABGESCHLOSSEN

---

## Build-Status

| Datum | Status | RAM | Flash |
|-------|--------|-----|-------|
| 2026-03-15 | ✅ SUCCESS | 16.5% | 33.5% |

## Gesamtergebnis

### Abgeschlossene Phasen

| Phase | Beschreibung | Status |
|-------|--------------|--------|
| 1.1 | Heap-Fragmentierung (String → char-Buffer) | ✅ |
| 1.2 | EEPROM → NVS (Wear-Leveling) | ✅ |
| 2 | Thread-Sicherheit (std::atomic) | ✅ |
| 3 | DHCP statt statischer IP | ✅ |
| 4 | Modularisierung (4 Module) | ✅ |
| 5 | Hardware-Sicherheits-Warnung | ✅ |

### Neue Dateien

| Datei | Größe | Zweck |
|-------|-------|-------|
| `src/Config.h` | ~10 KB | Konstanten, Pins, Enums, Settings |
| `src/HoverboardComm.h/cpp` | ~8 KB | UART Kommunikation |
| `src/SensorManager.h/cpp` | ~3 KB | Spannungsmessung |
| `src/NetworkManager.h/cpp` | ~4 KB | WiFi AP/STA |

### Commits

| Hash | Beschreibung |
|------|--------------|
| `f53bc71` | Phase 4.5: NetworkManager - Phase 4 ABGESCHLOSSEN |
| `bb82d44` | Phase 4.4: SensorManager |
| `cc11ed7` | Phase 4.3: HoverboardComm |
| `3a34dff` | Phase 4.2: Config.h eingebunden |
| `7f37a5c` | Phase 5: Hardware-Sicherheits-Warnung |
| `35e39eb` | Phase 3: DHCP |
| `f2bd406` | Phase 2: Thread-Sicherheit |
| `4e9c79e` | Phase 1.1: Heap-Fragmentierung |
| `7e714c6` | Phase 1.2: EEPROM → NVS |

---

## Commit-History

| Commit | Datum | Beschreibung |
|--------|-------|--------------|
| _ | _ | _ |
