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

## Phase 2: FreeRTOS Thread-Sicherheit 🔴 KRITISCH

**Problem:** Globale Variablen werden von mehreren Tasks ohne Schutz verwendet.

#### Schritt 2.1: Atomare Typen einführen
- [ ] `#include <atomic>` hinzufügen
- [ ] `volatile bool skill8SafetyActive` → `std::atomic<bool> skill8SafetyActive{false}`
- [ ] `volatile int currentBladeState` → `std::atomic<int> currentBladeState{BLADE_OFF}`
- [ ] `volatile bool isInSafetyMode` → `std::atomic<bool> isInSafetyMode{false}`
- [ ] `volatile bool hoverboardIsBusy` → `std::atomic<bool> hoverboardIsBusy{false}`
- [ ] **AKTUALISIERT:** _

#### Schritt 2.2: Komplexe Strukturen schützen
- [ ] Prüfen ob `feedbackMutex` ausreicht
- [ ] Ggf. weitere Mutexe für Shared Data
- [ ] **AKTUALISIERT:** _

---

## Phase 3: Netzwerk-Flexibilität (DHCP) 🟡 WICHTIG

**Problem:** Statische IP 192.168.1.123 funktioniert nicht in allen Netzwerken.

#### Schritt 3.1: DHCP als Standard
- [ ] `WiFi.config()` aus `connectToWiFi()` entfernen
- [ ] DHCP-Modus aktivieren
- [ ] **AKTUALISIERT:** _

#### Schritt 3.2: Optionale statische IP (Web-Interface)
- [ ] Settings-Struct erweitern (optionalIP, optionalGateway, optionalSubnet)
- [ ] Web-Interface erweitern
- [ ] Logik: Falls Werte gesetzt → statisch, sonst DHCP
- [ ] **AKTUALISIERT:** _

---

## Phase 4: Architektur & Modularisierung 🟢 LANGFRISTIG

**Ziel:** main.cpp entlasten, "God Class" auflösen.

#### Schritt 4.1: Config.h erstellen
- [ ] Alle `#define` aus main.cpp extrahieren
- [ ] Pin-Definitionen verschieben
- [ ] Settings-Struct verschieben
- [ ] **AKTUALISIERT:** _

#### Schritt 4.2: HoverboardComm.h/cpp erstellen
- [ ] UART-Kommunikation extrahieren
- [ ] `Send()`, `Receive()` verschieben
- [ ] Mutex-Handling kapseln
- [ ] **AKTUALISIERT:** _

#### Schritt 4.3: SensorManager.h/cpp erstellen
- [ ] MPU6050-Initialisierung verschieben
- [ ] `mpuReadTask` verschieben
- [ ] Spannungsmessung verschieben
- [ ] **AKTUALISIERT:** _

#### Schritt 4.4: NetworkManager.h/cpp erstellen
- [ ] WiFi-Setup verschieben
- [ ] WebServer verschieben
- [ ] DNS/AP-Handling verschieben
- [ ] **AKTUALISIERT:** _

---

## Phase 5: Hardware-Sicherheit (Not-Aus) 🔴 SICHERHEITSKRITISCH

**Problem:** E-Stop ist nur Software-basiert.

#### Schritt 5.1: Warnkommentar hinzufügen
- [ ] Kommentar in E-Stop Skill einfügen
- [ ] Hardware-Verdrahtung dokumentieren
- [ ] **AKTUALISIERT:** _

---

## Build-Status

| Datum | Status | RAM | Flash |
|-------|--------|-----|-------|
| _ | _ | _ | _ |

---

## Commit-History

| Commit | Datum | Beschreibung |
|--------|-------|--------------|
| _ | _ | _ |
