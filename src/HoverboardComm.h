/*
 * HoverboardComm.h
 * 
 * Phase 4.3: Hoverboard UART Kommunikation
 * Kapselt Send(), Receive() und alle zugehörigen Datenstrukturen
 */

#ifndef HOVERBOARDCOMM_H
#define HOVERBOARDCOMM_H

#include <Arduino.h>
#include <atomic>
#include <HardwareSerial.h>

// ====================================================================================
// ===== DATENSTRUKTUREN ===============================================================
// ====================================================================================

// Serial Command Struktur (Hoverboard-Protokoll)
typedef struct __attribute__((packed)) {
    uint16_t start;      // 0xABCD: Start-Kennung
    int16_t cmdCode;     // 1 = Normaler Joystick-Modus, 2 = Distanz-Mission
    int16_t steer;       // Modus 1: Lenkung (-1000 bis 1000) | Modus 2: Distanz Links (in cm)
    int16_t speed;       // Modus 1: Speed (-1000 bis 1000)   | Modus 2: Distanz Rechts (in cm)
    int16_t maxSpeedL;   // maxSpeedL
    int16_t maxSpeedR;   // maxSpeedR 
    int16_t accPct;      // Beschleunigungs-Zone: % der Gesamtstrecke (0-100)
    int16_t brkPct;      // Brems-Zone: % der Gesamtstrecke (0-100)
    uint16_t checksum;   // XOR-Prüfsumme über alle vorherigen Felder
} SerialCommand;

// Serial Feedback Struktur (Hoverboard-Antwort)
typedef struct __attribute__((packed)) {
    uint16_t start;
    int16_t cmd1;
    int16_t cmd2;
    int16_t speedR_meas;
    int16_t speedL_meas;
    int16_t batVoltage;
    int16_t boardTemp;
    uint16_t cmdLed;
    uint16_t checksum;
} SerialFeedback;

// ====================================================================================
// ===== GLOBALE VARIABLEN (extern - definiert in .cpp) ================================
// ====================================================================================

// UART Interface (muss von main.cpp gesetzt werden)
extern HardwareSerial* HoverSerial;

// Command & Feedback
extern SerialCommand Command;
extern SerialFeedback Feedback;
extern SerialFeedback NewFeedback;

// Globale Steuerungsvariablen
extern uint16_t global_cmdCode;
extern uint16_t global_accel;
extern uint16_t global_brake;
extern int16_t global_maxSpeedL;
extern int16_t global_maxSpeedR;

// Status Variablen
extern std::atomic<bool> hoverboardIsBusy;
extern volatile int16_t feedbackDistL;
extern volatile int16_t feedbackDistR;

// Mutexe für Thread-Sicherheit
extern SemaphoreHandle_t bufferMutex;
extern SemaphoreHandle_t feedbackMutex;

// Hilfsvariablen für Receive()
extern uint8_t idx;
extern uint16_t bufStartFrame;
extern byte *p;
extern byte incomingByte;
extern byte incomingBytePrev;

// ====================================================================================
// ===== FUNKTIONEN ====================================================================
// ====================================================================================

// Initialisierung
void initHoverboardComm(HardwareSerial* serial);

// HINWEIS: Send() und Receive() sind in main.cpp verblieben (komplexe Logik),
// verwenden aber die globalen Variablen aus diesem Modul.
// Deklarationen sind in main.cpp oder hier als extern:
extern void Send(int16_t uSteer, int16_t uSpeed);
extern void Receive();

// Sicheres Lesen des Feedbacks (mit Mutex-Schutz)
bool getFeedbackSafe(SerialFeedback& outFeedback);

// Prüft ob Feedback-Daten verfügbar sind
bool hasFeedbackData();

#endif // HOVERBOARDCOMM_H
