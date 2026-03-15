🛠️ Technische Anleitung: Bugfixes & Echte Modularisierung

Teil 1: Thread-Sicherheit für den Log-Ringpuffer (Spinlock)

Da beide Prozessorkerne (Core 0 und Core 1) potenziell gleichzeitig Text über debugPrintln() in den Puffer schreiben wollen, kann der Ringpuffer korrumpieren. Für so kleine, schnelle Operationen nutzt man in FreeRTOS einen Spinlock (portMUX_TYPE), da dieser schneller als ein normaler Mutex ist und auch in Interrupts (ISRs) funktioniert.

Schritt-für-Schritt:

✅ Task 1.1 ABGESCHLOSSEN: Spinlock deklariert
Zeitstempel: 2026-03-15 06:45 CET
Was gemacht wurde:
- portMUX_TYPE logMutex = portMUX_INITIALIZER_UNLOCKED; in main.cpp hinzugefügt
- Position: Nach den HoverboardComm extern-Deklarationen, vor den Funktionsdefinitionen
- Der Spinlock ist global sichtbar und wird von addToWebLogBuffer und statusTask genutzt
✅ Task 1.2 ABGESCHLOSSEN: Schreibzugriff in addToWebLogBuffer() abgesichert
Zeitstempel: 2026-03-15 06:47 CET
Was gemacht wurde:
- portENTER_CRITICAL(&logMutex) am Funktionsanfang hinzugefügt (nach Null-Checks)
- portEXIT_CRITICAL(&logMutex) am Funktionsende hinzugefügt (vor return)
- Schützt: webLogWriteIndex, webLogBuffer[], webLogOverflow
- Wichtig: Nur der kritische Bereich (Puffer-Manipulation) ist gesperrt, nicht die ganze Funktion
✅ Task 1.3 ABGESCHLOSSEN: Lesezugriff in getFromWebLogBuffer() abgesichert
Zeitstempel: 2026-03-15 06:49 CET
Was gemacht wurde:
- portENTER_CRITICAL(&logMutex) am Funktionsanfang hinzugefügt
- portEXIT_CRITICAL(&logMutex) am Funktionsende hinzugefügt
- Schützt: webLogReadIndex, webLogBuffer[], webLogOverflow
- Wichtig: Der WebSocket-Task (Core 0) kann jetzt nicht mehr gleichzeitig lesen,
  während der Logging-Task (Core 1) schreibt

Teil 1 ZUSAMMENFASSUNG:
Alle Zugriffe auf den Ringpuffer sind jetzt durch portMUX_TYPE geschützt.
Race Conditions beim gleichzeitigen Schreiben/Lesen sind eliminiert.

Teil 2: Echte Modularisierung (Raus aus der main.cpp)

Das Problem beim Verschieben von Receive() und Send() in die HoverboardComm.cpp ist, dass diese Funktionen auf globale Variablen aus der main.cpp zugreifen (z.B. hoverboardIsBusy, input_EspNowSteer, currentSettings).

Schritt-für-Schritt (Der "Extern"-Weg):
Um die Funktionen wirklich verschieben zu können, müssen die Module wissen, dass diese Variablen existieren.

✅ Task 2.1 ABGESCHLOSSEN: Globals.h erstellt
Zeitstempel: 2026-03-15 06:55 CET
Was gemacht wurde:
- Neue Datei src/Globals.h erstellt
- Alle relevanten globalen Variablen als extern deklariert:
  * Skill & State Variablen (skill8Active, monitorDirection, etc.)
  * Steuerungs-Variablen (skillActive, currentSkill, skillSteer)
  * Input Variablen (input_EspNowSpeed/Steer, input_JoySpeed/Steer)
  * Safety Variablen (isInSafetyMode, skill8SafetyActive, etc.)
  * Network Variablen (isWiFiConnected, apStartTime, etc.)
  * Logging Variablen (webLogActive, webLogBuffer, etc.)
- Forward Declarations für wichtige Funktionen hinzugefügt
- Datei includet Config.h und HoverboardComm.h für Abhängigkeiten
Dateien verschieben (HoverboardComm.cpp):
Nimm den kompletten Code von Receive() und Send() (und die Deklaration von SerialFeedback etc.) und schneide ihn aus der main.cpp aus. Füge ihn in HoverboardComm.cpp ein.

C++
// HoverboardComm.cpp
#include "HoverboardComm.h"
#include "Globals.h" // Jetzt kennt die Datei hoverboardIsBusy etc.

void Receive() {
    // ... dein bisheriger Code ...
}
Dateien verschieben (NetworkManager.cpp):
Schneide core1WiFiTask, onWsEvent, setupWiFiAP und connectToWiFi aus der main.cpp aus und füge sie in NetworkManager.cpp ein. Auch hier: #include "Globals.h" oben einfügen.

main.cpp aufräumen:
In der main.cpp bleiben diese Variablen nun einmalig als echte Definitionen stehen (ohne extern):

C++
// main.cpp
std::atomic<bool> hoverboardIsBusy{false};
std::atomic<int16_t> input_EspNowSteer{0};
// ...
Die main.cpp includiert dann nur noch #include "HoverboardComm.h" und #include "NetworkManager.h" und ruft im Loop/Setup die Tasks auf.

❓ Meine Rückfragen an dich:

Um dir den genauen, fertigen Code generieren zu können, muss ich kurz wissen, wie du die Architektur am liebsten hättest:

Objektorientiert (Klassen) vs. Prozedural (Extern-Variablen): Sollen wir die extern-Methode nutzen (wie oben beschrieben, das ist einfacher und näher an deinem jetzigen Code) oder soll ich daraus saubere C++ Klassen bauen (z.B. Hoverboard hoverboard; hoverboard.receive();), was moderner ist, aber mehr Umbau erfordert?

Der Ringpuffer:
Hast du den neuen Ringpuffer (ohne String) schon selbst geschrieben und er braucht nur noch den Mutex,