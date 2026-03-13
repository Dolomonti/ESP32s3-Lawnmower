# 📋 MASTER TO-DO-LISTE: ESP32 Lawnmower Refactoring

> **Letzte Aktualisierung:** 2026-03-13  
> **Plan erstellt von:** Kimi (Plan Mode)  
> **Detaillierter Plan:** [`.github/plans/refactoring-plan.md`](./plans/refactoring-plan.md)

## Übersicht für den Boss

| Phase | Kritisch | Mittel | Niedrig | Nicht nötig |
|-------|----------|--------|---------|-------------|
| Phase 1 | 2 | 0 | 0 | 1 |
| Phase 2 | 2 | 1 | 0 | 0 |
| Phase 3 | 0 | 2 | 0 | 1 |
| Phase 4 | 0 | 0 | 3 | 0 |
| **Gesamt** | **4** | **3** | **3** | **2** |

**Legende Tracking-Tabelle:**
- ⬜ Offen - Noch nicht begonnen
- 🔄 In Arbeit - Wird gerade implementiert
- ✅ Erledigt - Implementiert und getestet
- ❌ Nicht nötig - Technisch nicht sinnvoll

---

Hier ist die Master-To-Do-Liste. Sie dient als exakter, technischer Fahrplan (Execution Plan) für Kimi (oder dich). Jeder Task ist in Teilschritte zerlegt, enthält die genaue technische Begründung und berücksichtigt alle strikten Architekturvorgaben (wie den Erhalt der `Send`-Parameter und den Standby-Status des Blade-Bypass).

---

# 🛠️ SENIOR ARCHITECT MASTER TO-DO LISTE: ESP32 Lawnmower

**Erklärung zur Liste:**
Dies ist das technische Backlog zur Härtung der `main.cpp` und zur Beseitigung der "Vibe-Coder-Entropie". Jeder Task ist in atomare Teilschritte unterteilt. Code-Änderungen dürfen nur strikt nach diesen Vorgaben vorgenommen werden. Die Architektur (insbesondere das Trennen von Mission und Joystick sowie die Trennung von Kern 0 und Kern 1) muss unangetastet bleiben.

---

### 🏁 PHASE 0: Analyse & Projekt-Setup

~~**[x] Task 0.1: Codebase analysieren und Architektur-Schwachstellen identifizieren**~~
*Detail: Vollständiger Review der `main.cpp`, `webpage.cpp` und Hardware-Konstanten.*

> *Erledigt: Tiefenanalyse durchgeführt. Kritische Race-Conditions (I2C), Variablen-Shadowing und Architektur-Leaks in der `Send()` Funktion erkannt. Vorgaben des Users (Send-Struktur erhalten, Blade-Bypass auslassen) in den Plan integriert.*

~~**[x] Task 0.2: Master-To-Do-Liste und Leitplanken (Guards) definieren**~~
*Detail: Erstellung eines strikten, sequenziellen Abarbeitungsplans.*

> *Erledigt: Dieses Dokument wurde als "Single Source of Truth" für das Refactoring generiert.*

---

### 🚨 PHASE 1: Kritische Fehler & Stabilität (Prio: Hoch)

**[ ] Task 1.1: I2C Race Conditions beheben (Core 0)** | **NICHT UMSETZEN** - Technisch falsch, analogRead() stört I2C nicht

* **Technische Erklärung:** `mpuReadTask` und `statusTask` laufen beide auf Core 0. Der `statusTask` führt `analogRead` aus, was je nach ESP-IDF Version Hardware-Interrupts auslöst, die den I2C-Bus (MPU6050) stören können.
* **Teilschritte:**
* [ ] Finde in `statusTask` den Aufruf `analogReadMilliVolts(BLADE_BATTERY_PIN)`.
* [ ] Umschließe diesen und alle anderen ADC/Hardware-Reads strikt mit `xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(10))` und `xSemaphoreGive(i2cMutex)`.
* [ ] Prüfe `mpuReadTask` auf konsistente Nutzung desselben Mutex.



**[ ] Task 1.2: Task Watchdog (WDT) Triggers verhindern**

* **Technische Erklärung:** Längere Blockaden (z.B. in `while`-Schleifen für WiFi oder beim String-Parsing) triggern den Hardware-Watchdog des ESP32, was zu zufälligen Reboots führt.
* **Teilschritte:**
* [ ] Analysiere `connectToWiFi()`: Füge `vTaskDelay(10 / portTICK_PERIOD_MS)` in die `while (WiFi.status() != WL_CONNECTED)` Schleife ein.
* [ ] Prüfe `handleSerialInput()` und `Receive()` auf fehlende Delays in großen `while`-Blöcken.

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| vTaskDelay(500) bereits vorhanden in connectToWiFi() | - | - | ⬜ Nicht nötig |



**[ ] Task 1.3: SerialFeedback Struct Sicherheit (Buffer Overflow)**

* **Technische Erklärung:** Die Prüfsummenberechnung in `Receive()` nutzt Pointer-Arithmetik (`memcpy(&Feedback, &NewFeedback, sizeof(SerialFeedback))`) auf unvalidierte Serielle Daten.
* **Teilschritte:**
* [ ] Stelle sicher, dass `__attribute__((packed))` bei `SerialFeedback` vom Compiler exakt respektiert wird (Längenprüfung `if (sizeof(SerialFeedback) == 18)` ins `setup()` einbauen).
* [ ] Implementiere einen Guard, der das `memcpy` nur ausführt, wenn `idx` exakt der Struct-Größe entspricht, um Memory Corruption zu vermeiden.

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| sizeof(SerialFeedback)==18 Check in setup() | 2026-03-13 | - | ✅ Erledigt |
| Guard: if(idx==sizeof(SerialFeedback)) in Receive() | - | - | ✅ Bereits vorhanden |



**[ ] Task 1.4: Konfliktlösung E-Stop vs. Mission (Skill 20)**

* **Technische Erklärung:** Wenn Skill 20 (Distanzmission) läuft, wartet `controlLogicTask` auf das Hoverboard (`hoverboardIsBusy`). Ein E-Stop am UI setzt `skill8SafetyActive = true`, aber die Task ignoriert das eventuell, weil sie im Missions-State festhängt.
* **Teilschritte:**
* [ ] In `controlLogicTask`: Füge ganz oben in die Missionslogik (`if (skillActive && currentSkill == 20)`) eine harte Abbruchbedingung ein: `if (skill8SafetyActive) { resetAll(); skillActive = false; ... }`.

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| E-Stop Check in controlLogicTask Skill 20 Block | 2026-03-13 | - | ✅ Erledigt |
| Test: E-Stop während Mission | - | Boss | ⬜ Offen |



---

### 🏗️ PHASE 2: Architektur-Fixes & Variablen-Hygiene (Prio: Hoch)

**[ ] Task 2.1: Refactoring der `Send()` Funktion (Struktur zwingend erhalten!)**

* **Technische Erklärung:** Die Signatur `Send(int16_t uSteer, int16_t uSpeed)` ist architektonisch korrekt und muss bleiben. Aktuell überschreibt die Funktion diese Parameter intern mit globalen Variablen, falls `global_cmdCode == 2` ist. Das ist ein Design-Fehler.
* **Teilschritte:**
* [ ] *Code säubern:* Lösche die "tote" Forward-Declaration `void Send(int16_t uSteer, int16_t uSpeed);` (ca. Zeile 647, mitten im Code).
* [ ] *Logik bereinigen:* Entferne die `if (global_cmdCode == 2)` Weiche *innerhalb* von `Send()`. Die Funktion darf `Command.steer` und `Command.speed` nur noch stumpf mit den übergebenen Parametern `uSteer` und `uSpeed` füllen.
* [ ] *Verantwortung verschieben:* Passe `controlLogicTask` an. Dort muss vor dem Aufruf von `Send(...)` entschieden werden, ob `lastEspNowSteer/Speed` (Mission) oder die Joystick-Werte übergeben werden.

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| Forward-Declaration entfernt | 2026-03-13 | - | ✅ Erledigt |
| if(global_cmdCode==2) aus Send() entfernt | 2026-03-13 | - | ✅ Erledigt |
| controlLogicTask angepasst | 2026-03-13 | - | ✅ Erledigt (war bereits korrekt) |
| Build-Test erfolgreich | 2026-03-13 | - | ✅ Erledigt |



**[ ] Task 2.2: Variable Shadowing & Redundanz auflösen**

* **Technische Erklärung:** Variablen wie `global_maxSpeedL`, `lastEspNowSpeed`, `joystickSpeed` überschneiden sich in ihrer Funktion.
* **Teilschritte:**
* [ ] Konsolidiere die globalen Variablen am Dateianfang (`main.cpp`).
* [ ] Trenne saubere Structs oder Präfixe für Input-Quellen: z.B. `input_JoySteer`, `input_JoySpeed` vs. `mission_DistL`, `mission_DistR`.
* [ ] Ersetze alle losen `#define MAX_SPEED` Nutzungen im Code durch die EEPROM-gestützten Werte `currentSettings.maxSpeed`.

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| Naming-Convention definiert | 2026-03-13 | - | ✅ Erledigt |
| Variablen umbenannt | 2026-03-13 | - | ✅ Erledigt (lastEspNow* → input_EspNow*) |
| Alle Referenzen aktualisiert | 2026-03-13 | - | ✅ Erledigt |
| Build-Test erfolgreich | 2026-03-13 | - | ✅ Erledigt |



**[ ] Task 2.3: Typen-Konflikte (Voltage Scaling) beheben**

* **Technische Erklärung:** `bladeBatteryFactor` ist ein `float`. Berechnungen in `handleSystemStatus` konvertieren wild zwischen `float` und `int32_t` hin und her, was bei Grenzwerten (z.B. Abschaltung bei 31.0V) zu Fehlern führt.
* **Teilschritte:**
* [ ] In `statusTask`: Stelle sicher, dass `smoothed_blade_mv` explizit und sauber auf `int32_t` gerundet wird, bevor es in `global_blade_voltage_mv` landet.
* [ ] In `handleSystemStatus`: Caste alle Schwellenwerte (`currentSettings.bladeMinShutdownVoltage`) explizit und vergleiche nur Integer mit Integer (Millivolt mit Millivolt).

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| Explizite Casts in statusTask | 2026-03-13 | - | ✅ Erledigt |
| Integer-Vergleiche in handleSystemStatus | 2026-03-13 | - | ✅ Bereits korrekt |
| Test mit Spannungsgrenzwerten | - | Boss | ⬜ Offen |



---

### 🌐 PHASE 3: Web-UI & Speicher-Sicherheit (Prio: Mittel)

**[ ] Task 3.1: JSON Stack Overflow Gefahr in WebSockets beseitigen**

* **Technische Erklärung:** `DynamicJsonDocument doc(512)` wird in `onWsEvent` lokal auf dem Stack allokiert. Bei größeren Payloads (z.B. AI Chat-Prompt) crasht der ESP32 Stack auf Core 1.
* **Teilschritte:**
* [ ] Ersetze lokales `DynamicJsonDocument` durch ein am Heap allokiertes `DynamicJsonDocument` (Pointer) oder vergrößere es signifikant und mache es global/static (`StaticJsonDocument<2048>`), falls der RAM es zulässt.

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| **NICHT UMSETZEN** | - | - | ❌ Nicht nötig - 512 Bytes sind unkritisch für ESP32-S3 |



**[ ] Task 3.2: EEPROM Initialisierung härten**

* **Technische Erklärung:** Die Prüfung auf `0xDEADBEEF` als Magic Number ist gut. Bei einem brandneuen Chip können aber undefinierte Werte im Flash stehen, die den Rest des Structs korrumpieren, wenn der CRC fehlschlägt.
* **Teilschritte:**
* [ ] In `loadSettings()`: Wenn der CRC-Check fehlschlägt, rufe zwingend `memset(&currentSettings, 0, sizeof(Settings))` auf, bevor die Default-Werte zugewiesen werden.

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| memset() vor Default-Zuweisung | 2026-03-13 | - | ✅ Erledigt |
| Test mit gelöschtem EEPROM | - | Boss | ⬜ Offen |



**[ ] Task 3.3: Web-Command ACK (Missing Feedback)**

* **Technische Erklärung:** Das Web-UI sendet Befehle an `/ws`, bekommt aber bei Mutex-Blockaden oder Fehlern kein direktes asynchrones Feedback vom ESP.
* **Teilschritte:**
* [ ] In `onWsEvent` (Code 999 und Skills): Sende via `client->text("CMD: ... [OK/FAILED]")` eine garantierte Bestätigung zurück an den Browser, die im Log-Fenster angezeigt wird.

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| ACK für Skill-Befehle | - | - | ✅ Bereits implementiert |
| ACK für Code 999 | - | - | ✅ Bereits implementiert |
| Test im Web-UI Log | - | Boss | ⬜ Offen |



---

### 🧹 PHASE 4: Code-Hygiene (Prio: Niedrig)

**[ ] Task 4.1: Doppelte Hardware-Deklarationen entfernen**

* **Teilschritte:**
* [ ] In `setup()`: Suche nach `pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP)` und entferne die doppelte Aufrufe (steht aktuell zweimal direkt untereinander).

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| Doppelten pinMode() entfernt | 2026-03-13 | - | ✅ Erledigt |



**[ ] Task 4.2: Template Overloads für Serial Debugging**

* **Teilschritte:**
* [ ] Füge in `main.cpp` oben (bei den `debugPrint` Templates) explizite Overloads für `float` hinzu, um String-Casting-Fehler bei Logging-Ausgaben zu vermeiden: `void debugPrint(float msg)` und `debugPrintln(float msg)`.

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| debugPrint(float) overload | 2026-03-13 | - | ✅ Erledigt |
| debugPrintln(float) overload | 2026-03-13 | - | ✅ Erledigt |
| Test mit Float-Ausgaben | - | Boss | ⬜ Offen |



**[ ] Task 4.3: Refactoring des "Spaghetti-Monsters" (`triggerSkill`)**

* **Teilschritte:**
* [ ] Unterteile den riesigen `switch-case` in `triggerSkill()` in kleinere Sub-Funktionen: `handleDriveSkills(skill, p1...)`, `handleBladeSkills(...)`, `handleSetupSkills(...)`, um die Wartbarkeit zu erhöhen.

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| handleDriveSkills() erstellt | 2026-03-13 | - | ✅ Erledigt |
| handleBladeSkills() erstellt | 2026-03-13 | - | ✅ Erledigt |
| handleSetupSkills() erstellt | 2026-03-13 | - | ✅ Erledigt |
| triggerSkill() refactored | 2026-03-13 | - | ✅ Erledigt |
| Build-Test erfolgreich | 2026-03-13 | - | ✅ Erledigt |



---

### ⏸️ PHASE 5: STANDBY / BACKLOG (Aktuell blockiert/Abgebrochen)

~~**[-] Task 5.1: Blade-Safety Hardware-Bypass**~~

* **Technische Erklärung:** In `runBladeLogic()` sollte der PWM-Output hart genullt werden, wenn das Messer (`currentBladeHeight`) physisch nicht unten ist (Hardware-Feedback-Loop).
* **Teilschritte:**
* ~~Lese Zustand des physischen Endschalters ein.~~
* ~~Überschreibe `current_blade_pwm` mit `BLADE_ZERO_US` vor `bladeEsc.write`.~~



> *Status: ABGEBROCHEN / PAUSIERT. Anweisung vom User: Dieses Feature wird später gebaut und bleibt aktuell auf Standby. Der Bereich in `runBladeLogic()` darf im aktuellen Sprint von Kimi nicht angefasst werden.*