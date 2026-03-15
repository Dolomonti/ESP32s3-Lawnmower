# 📋 MASTER TO-DO-LISTE: ESP32 Lawnmower Refactoring

> **Letzte Aktualisierung:** 2026-03-13  
> **Plan erstellt von:** Kimi (Plan Mode)  
> **Detaillierter Plan:** [`.github/plans/refactoring-plan.md`](./plans/refactoring-plan.md)

## Übersicht für den Boss

| Phase | Kritisch | Mittel | Niedrig | Nicht nötig |
|-------|----------|--------|---------|-------------|
| Phase 1 | 3 | 0 | 0 | 0 |
| Phase 2 | 2 | 0 | 0 | 0 |
| Phase 3 | 0 | 0 | 1 | 0 |
| **Gesamt** | **5** | **0** | **1** | **0** |

**Legende Tracking-Tabelle:**
- ⬜ Offen - Noch nicht begonnen
- 🔄 In Arbeit - Wird gerade implementiert
- ✅ Erledigt - Implementiert und getestet
- ❌ Nicht nötig - Technisch nicht sinnvoll

---

Hier ist die Master-To-Do-Liste. Sie dient als exakter, technischer Fahrplan (Execution Plan) für Kimi (oder dich). Jeder Task ist in Teilschritte zerlegt, enthält die genaue technische Begründung und berücksichtigt alle strikten Architekturvorgaben.

---

# 🛠️ SENIOR ARCHITECT MASTER TO-DO LISTE: ESP32 Lawnmower

### 🚨 PHASE 1: Kritische Fehler & Stabilität (Prio: Hoch)

**[ ] Task 1.1: Race Condition in der UART-Kommunikation beheben (`Send()`)**

* **Technische Erklärung:** Die globale Struktur `Command` wird asynchron von mehreren Cores/Tasks modifiziert und über UART gesendet (`HoverSerial.write`), was zu Paketkollisionen und Abstürzen führt.
* **Teilschritte:**
  * [ ] Finde die Funktion `Send(int16_t uSteer, int16_t uSpeed)`.
  * [ ] Identifiziere den Block mit den Zuweisungen an `Command` und `HoverSerial.write`.
  * [ ] Umschließe diesen Block komplett mit `if (xSemaphoreTake(bufferMutex, (TickType_t)10) == pdTRUE) { ... xSemaphoreGive(bufferMutex); }`.

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| Mutex in Send() implementieren | 2026-03-13 | - | ✅ Erledigt |

**[ ] Task 1.2: Memory Corruption im AsyncWebServer beheben (`onWsEvent`)**

* **Technische Erklärung:** Ein statisch deklariertes JSON-Dokument in einem asynchronen Web-Handler teilt seinen Speicher über Threads hinweg. Gleichzeitige WebSocket-Nachrichten (z.B. Joystick) verursachen Memory Corruption und System-Panics.
* **Teilschritte:**
  * [ ] Finde in `onWsEvent` die Zeile: `static DynamicJsonDocument doc(512);`.
  * [ ] Entferne das Keyword `static` -> `DynamicJsonDocument doc(512);`.
  * [ ] Lösche die darauffolgende Zeile `doc.clear();`.

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| static Keyword & clear() entfernen | 2026-03-13 | - | ✅ Erledigt |

**[ ] Task 1.3: Blockierenden Code in Async-Callbacks entfernen (`resetDevice`)**

* **Technische Erklärung:** `delay()` und direkter Neustart (`ESP.restart()`) innerhalb des Web-Server-Threads blockieren den TCP-Stack und lösen den Hardware-Watchdog aus.
* **Teilschritte:**
  * [ ] Finde die Funktion `resetDevice(AsyncWebServerRequest *request)`.
  * [ ] Lösche `delay(200);` und `ESP.restart();`.
  * [ ] Füge `shouldRestart = true;` als sicheren Flag-Trigger ein.

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| Blockierenden Code durch Flag ersetzen | 2026-03-13 | - | ✅ Erledigt |

---

### 🏗️ PHASE 2: Architektur-Fixes & Logikfehler (Prio: Hoch)

**[ ] Task 2.1: Fehlende Limitierung (Constrain) für Web-Joystick Eingaben**

* **Technische Erklärung:** Bei eingehenden WebSocket-Befehlen (`msgType == "joystick"`) werden die globalen Variablen ohne Überprüfung der Systemlimits beschrieben. Der Schutzmechanismus aus dem Serial-Monitor fehlt hier.
* **Teilschritte:**
  * [ ] Finde in `onWsEvent` den Block: `else if (strcmp(msgType, "joystick") == 0)`.
  * [ ] Limitiere `rawSteer` und `rawSpeed` mittels `constrain(..., -currentSettings.currentMaxSteer, currentSettings.currentMaxSteer)`.
  * [ ] Weise die geschützten Werte `input_EspNowSteer` und `input_JoySteer` zu.

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| constrain() auf Joystick-Werte anwenden | 2026-03-13 | - | ✅ Erledigt |

**[ ] Task 2.2: IP-Subnetz Konflikt im Heimnetzwerk (`connectToWiFi`) beheben**

* **Technische Erklärung:** Das Gerät versucht, eine statische IP im `192.168.1.x` Netz aufzubauen, obwohl das Zielnetzwerk (laut Doku/Setup) ein FritzBox-Netzwerk im Bereich `192.168.178.x` ist.
* **Teilschritte:**
  * [ ] Finde in `connectToWiFi` die Definition von `sta_local_IP` und `sta_gateway`.
  * [ ] Ändere das Subnetz in den IPs von `1` auf `178` (z.B. `192, 168, 178, 123`).

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| Subnetz auf 178.x ändern | 2026-03-13 | - | ❌ Zurückgerollt (auf 1.x belassen) |

---

### 🧹 PHASE 3: Code-Hygiene & Toter Code (Prio: Niedrig)

**[ ] Task 3.1: Toten Code (Dead Code) und redundante Wrapper löschen**

* **Technische Erklärung:** Unnötiger Code verschlechtert die Lesbarkeit, verbraucht Speicherplatz und sorgt für "Vibe-Coder-Entropie".
* **Teilschritte:**
  * [ ] Lösche die gesamte Funktion `void deactivateSkill()`.
  * [ ] Lösche die gesamte Funktion `void checkPeerTimeout()`.
  * [ ] Lösche die gesamte Funktion `void printOwnMacAddress()`.
  * [ ] Lösche die gesamte Funktion `void checkWebSocketConnection()`. Ersetze eventuelle Aufrufe direkt durch `ws.cleanupClients();`.

| Implementierung | Datum | Tester | Status |
|-----------------|-------|--------|--------|
| Tote Funktionen entfernt | 2026-03-13 | - | ✅ Erledigt |
| - deactivateSkill() | | | |
| - checkPeerTimeout() | | | |
| - printOwnMacAddress() | | | |
| - checkWebSocketConnection() | | | |