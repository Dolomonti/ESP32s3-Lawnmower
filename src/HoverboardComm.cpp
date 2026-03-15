/*
 * HoverboardComm.cpp
 * 
 * Phase 4.3: Hoverboard UART Kommunikation Implementierung
 */

#include "HoverboardComm.h"
#include "Config.h"

// ====================================================================================
// ===== GLOBALE VARIABLEN (Definitionen) ==============================================
// ====================================================================================

HardwareSerial* HoverSerial = nullptr;  // Wird in main.cpp auf &HoverSerialInstance gesetzt

SerialCommand Command;
SerialFeedback Feedback;
SerialFeedback NewFeedback;

uint16_t global_cmdCode = 0;
uint16_t global_accel = 10;
uint16_t global_brake = 100;
int16_t global_maxSpeedL = 0;
int16_t global_maxSpeedR = 0;

std::atomic<bool> hoverboardIsBusy{false};
volatile int16_t feedbackDistL = 0;
volatile int16_t feedbackDistR = 0;

SemaphoreHandle_t bufferMutex = nullptr;
SemaphoreHandle_t feedbackMutex = nullptr;

// Hilfsvariablen für Receive()
uint8_t idx = 0;
uint16_t bufStartFrame = 0;
byte *p = nullptr;
byte incomingByte = 0;
byte incomingBytePrev = 0;

// ====================================================================================
// ===== IMPLEMENTIERUNGEN =============================================================
// ====================================================================================

void initHoverboardComm(HardwareSerial* serial) {
    HoverSerial = serial;
    
    // Mutexe erstellen
    bufferMutex = xSemaphoreCreateMutex();
    feedbackMutex = xSemaphoreCreateMutex();
    
    // Structs initialisieren
    memset(&Command, 0, sizeof(SerialCommand));
    memset(&Feedback, 0, sizeof(SerialFeedback));
    memset(&NewFeedback, 0, sizeof(SerialFeedback));
}

// HINWEIS: Send() Funktion ist in main.cpp verblieben (hat zusätzliche Logik:
// Heartbeat-Limiting, High-Voltage-Shutdown, Skill8 Safety). Sie verwendet
// die globalen Variablen (Command, bufferMutex, etc.) aus diesem Modul.

// HINWEIS: Receive() Funktion ist in main.cpp verblieben (hat komplexe Zustands-
// maschinen-Logik). Sie verwendet die globalen Variablen aus diesem Modul.

bool getFeedbackSafe(SerialFeedback& outFeedback) {
    if (xSemaphoreTake(feedbackMutex, (TickType_t)5) == pdTRUE) {
        memcpy(&outFeedback, &Feedback, sizeof(SerialFeedback));
        xSemaphoreGive(feedbackMutex);
        return true;
    }
    return false;
}

bool hasFeedbackData() {
    // Einfache Prüfung ob Daten empfangen wurden (start != 0)
    return Feedback.start == 0xABCD;
}
