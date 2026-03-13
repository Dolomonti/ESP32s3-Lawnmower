
#include <Arduino.h>

/*
// ====================================================================================
// =================== BENUTZERHANDBUCH: ESP32 PS3 FERNSTEUERUNG ===================
// ====================================================================================
//
// Dieses Handbuch beschreibt alle Funktionen, Tastenbelegungen und
// Konfigurationsmöglichkeiten des Remote-Controller-Codes.
//
// ------------------------------------------------------------------------------------
// -- I. TASTENBELEGUNG (Standardfunktionen)
// ------------------------------------------------------------------------------------
//
// --- FAHRSTEUERUNG ---
// * Linker Analog-Stick (Vertikal):
//   - BESCHLEUNIGUNG (Gas geben / Rückwärtsfahren)
//   - Die maximale Geschwindigkeit wird durch den "Speed Mode" (siehe D-Pad) begrenzt.
//
// * Rechter Analog-Stick (Horizontal):
//   - LENKUNG (Links / Rechts)
//   - Die Lenkung wird durch den "Steering Trim" (siehe D-Pad) beeinflusst.
//
// * L3 (Linken Stick drücken):
//   - TEMPOMAT für Geschwindigkeit (Cruise Control)
//   - Drücken, um die aktuelle Geschwindigkeit zu halten. Erneutes Drücken zum Deaktivieren.
//
// * R3 (Rechten Stick drücken):
//   - TEMPOMAT für Lenkung (Cruise Control)
//   - Drücken, um den aktuellen Lenkwinkel zu halten. Erneutes Drücken zum Deaktivieren.
//
// --- SYSTEM & SPEZIALAKTIONEN ---
// * START-Taste:
//   - RESET-SEQUENZ
//   - Sendet einen Reset-Befehl (Code 9) an den Empfänger.
//   - Startet danach die Fernsteuerung (diesen ESP32) neu.
//
// * SELECT-Taste:
//   - STEUERUNGSMODUS WECHSELN (Lokal)
//   - Schaltet zyklisch durch die drei lokalen Steuerungsmodi (1 -> 2 -> 3 -> 1).
//   - Beeinflusst nur, wie die Stick-Eingaben lokal verarbeitet werden (linear, gestuft, etc.).
//
// * KREUZ (X)-Taste:
//   - SYSTEM-RESET am EMPFÄNGER
//   - Sendet einen Reset-Befehl für die Fahreinstellungen (Code 1) am Empfänger.
//   - Setzt ZUSÄTZLICH den lokalen Lenk-Trim auf 0 und deaktiviert beide Tempomaten.
//
// * DREIECK (▲)-Taste:
//   - GERADEAUSFAHRT HALTEN
//   - Sendet einen Befehl (Code 3), der die Lenkung auf 0 setzt und den Empfänger anweist,
//     die Spur zu halten (falls dort implementiert).
//
// --- KLINGENSTEUERUNG & MANÖVER ---
// * R1-Taste:
//   - KLINGE EIN
//   - Sendet den Befehl zum Einschalten der Klinge (Code 6, Wert 1).
//
// * R2-Taste:
//   - KLINGE AUS
//   - Sendet den Befehl zum Ausschalten der Klinge (Code 6, Wert -1).
//
// * QUADRAT (□)-Taste:
//   - 90°-DREHUNG VORBEREITEN
//   - Bereitet eine 90°-Drehung vor. Führe sie mit dem D-Pad (Links/Rechts) aus.
//
// * KREIS (O)-Taste:
//   - 180°-DREHUNG VORBEREITEN
//   - Bereitet eine 180°-Drehung vor. Führe sie mit dem D-Pad (Links/Rechts) aus.
//
// --- ANPASSUNGEN (D-PAD) ---
// * D-Pad Oben/Unten:
//   - GESCHWINDIGKEITSMODUS ÄNDERN
//   - Schaltet zwischen 4 Geschwindigkeitsstufen (1=langsam, 4=schnell).
//   - Jede Stufe entspricht 25% der MAX_SPEED (z.B. 25%, 50%, 75%, 100%).
//
// * D-Pad Links/Rechts:
//   - PRIMÄR: Vorbereitete Drehung ausführen (siehe Quadrat/Kreis).
//   - SEKUNDÄR (wenn keine Drehung vorbereitet ist): Lenk-Trim anpassen.
//     - Justiert die Mittelstellung der Lenkung. Nützlich, wenn das Fahrzeug nicht geradeaus fährt.
//
// --- SICHERHEITSFUNKTIONEN ---
//
// HINWEIS: Die Kipp-Schutz-Parameter (Winkel, Timeout) werden am Empfänger über Skill 14 gesetzt:
//   Beispiel: { "type": "command", "data": { "code": 14, "angle": 60, "timeout": 1500 } }
//   Diese Werte werden dauerhaft gespeichert und bestimmen, ab wann der Not-Aus (Skill 8) automatisch ausgelöst wird.
//
// * L1-Taste (Konfigurierbar):
//   - Modus 1: NOT-AUS (Standard)
//     - Einmaliges Drücken sendet einen sofortigen Stopp-Befehl.
//   - Modus 2: TOTMANNSCHALTER (Deadman Switch)
//     - Muss GEDRÜCKT GEHALTEN werden, damit Steuerbefehle gesendet werden.
//     - Lässt man L1 los, wird sofort ein Stopp-Befehl gesendet.
//
// * L2-Taste:
//   - RESET STEUERUNGSMODUS
//   - Setzt den lokalen Steuerungsmodus sofort auf Modus 1 (Lineare Steuerung) zurück.
//
// * PS-Taste:
//   - RESERVIERT
//   - Wird von der Bibliothek für das Pairing mit dem ESP32 verwendet. Keine eigene Funktion im Code.
//
// ------------------------------------------------------------------------------------
// -- II. KONFIGURATION IM CODE
// ------------------------------------------------------------------------------------
//
// Alle wichtigen Einstellungen können am Anfang des Codes als #define oder Konstanten
// angepasst werden.
//
// --- WICHTIG: VERBINDUNGSEINSTELLUNGEN ---
// * MAC-Adresse des Empfängers (für ESP-NOW):
//   uint8_t receiverMac[] = {0x48, 0x27, 0xE2, 0xE9, 0x11, 0x9C};
//   -> Ändere diese Adresse, damit sie mit der MAC-Adresse deines Empfänger-ESP32 übereinstimmt.
//
// * MAC-Adresse für PS3-Controller-Pairing:
//   Ps3.begin("20:00:00:00:01:20");
//   -> Diese Adresse ist die, auf die der PS3-Controller gepaart werden muss. Du musst
//      den Controller einmalig (z.B. mit SixaxisPairTool) auf DIESE MAC-Adresse einstellen,
//      damit er sich mit dem ESP32 verbindet.
//
// --- GRUNDEINSTELLUNGEN FAHRVERHALTEN ---
// * Maximale Geschwindigkeit:
//   #define MAX_SPEED 800
//   -> Der absolute Maximalwert für die Geschwindigkeit (vorwärts und rückwärts).
//
// * Maximale Lenkung:
//   #define MAX_STEER 1600
//   -> Der absolute Maximalwert für den Lenkausschlag.
//
// * Fahrtrichtung umkehren:
//   #define DRIVING_DIRECTION 1 // 1 = Normal, 2 = Invertiert
//   -> Kehrt die Reaktion des Gas-Sticks um (vorwärts/rückwärts).
//
// --- SICHERHEITSFUNKTION (L1-Taste) ---
// * L1 als Totmannschalter konfigurieren:
//   const bool L1_IS_DEADMAN_SWITCH = false;
//   -> `false` (Standard): L1 ist ein einmaliger Not-Aus-Knopf.
//   -> `true`: L1 muss zum Fahren gehalten werden (Totmannschalter).
//
// --- FEINABSTIMMUNG & MODI ---
// * Standard-Steuerungsmodus beim Start:
//   #define CONTROL_MODE 1
//   -> Setzt den Modus, der beim Start aktiv ist (1, 2 oder 3).
//
// * Schrittweite für Lenk-Trim:
//   #define TRIM_STEP 50
//   -> Wie stark jede Korrektur mit dem D-Pad die Lenkung beeinflusst.
//
// * Schwellenwerte für die Steuerungsmodi:
//   - Die Werte wie BELOW_THRESHOLD_1, MIDDLE_ZONE_2, etc. definieren die
//     "toten Zonen" und das Ansprechverhalten der Joysticks in den
//     verschiedenen Modi. Nur für Experten-Tuning anpassen.
//
// ------------------------------------------------------------------------------------
// -- III. STEUERUNGSMODI (LOKAL)
// ------------------------------------------------------------------------------------
// Die Modi werden mit der SELECT-Taste umgeschaltet. Sie ändern nur das Gefühl der
// Steuerung auf der Fernbedienung.
//
// * Modus 1: Lineare Steuerung (Standard)
//   - Die Stick-Position wird direkt und linear in Geschwindigkeit/Lenkung umgesetzt.
//   - 50% Stick-Ausschlag = 50% der max. Geschwindigkeit.
//   - Gut für ein direktes, vorhersehbares Fahrgefühl.
//
// * Modus 2: Gestufte Steuerung ("Three Fields Logic")
//   - Unterteilt den Stick-Weg in Zonen (z.B. eine langsame Zone für präzises
//     Manövrieren und eine schnelle Zone für Vollgas).
//   - Im Code aktuell nicht vollständig ausprogrammiert, verhält sich wie Modus 1.
//
// * Modus 3: Exponentielle Steuerung ("Threshold-based Acceleration")
//   - Kleine Stick-Bewegungen führen zu sehr feinen Änderungen.
//   - Große Stick-Bewegungen führen zu sehr schnellen Änderungen.
//   - Ideal für Aufgaben, die bei niedriger Geschwindigkeit hohe Präzision erfordern.
//
// ------------------------------------------------------------------------------------
// -- IV. STATUS-LED & HERZSCHLAG
// ------------------------------------------------------------------------------------
//
// * LED-Verhalten:
//   - LANGSAMES BLINKEN: Wartet auf Verbindung mit dem PS3-Controller.
//   - DAUERHAFT AN: PS3-Controller ist erfolgreich verbunden.
//
// * Herzschlag (Heartbeat):
//   - Wenn 5 Sekunden lang keine Taste gedrückt oder kein Stick bewegt wird,
//     sendet die Fernsteuerung automatisch einen "Herzschlag"-Befehl (Code 255).
//   - Dies signalisiert dem Empfänger, dass die Verbindung noch besteht.
//
// ====================================================================================
*/