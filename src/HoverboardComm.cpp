/*
 * HoverboardComm.cpp
 * 
 * Phase 4.3: Hoverboard UART Kommunikation Implementierung
 */

#include "HoverboardComm.h"
#include "Config.h"
#include "Globals.h"

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

// ====================================================================================
// ===== RECEIVE FUNKTION ==============================================================
// ====================================================================================
// WURDE AUS main.cpp VERSCHOBEN (To-Do 7: Vollständige Modularisierung)

void Receive() {
    if (!HoverSerial) return;
    
    while (HoverSerial->available()) {
        incomingByte = HoverSerial->read();
        bufStartFrame = ((uint16_t)(incomingByte) << 8) | incomingBytePrev;

        if (bufStartFrame == 0xABCD) {
            p = (byte *)&NewFeedback;
            *p++ = incomingBytePrev;
            *p++ = incomingByte;
            idx = 2;
        } else if (idx >= 2 && idx < sizeof(SerialFeedback)) {
            *p++ = incomingByte;
            idx++;
        }

        if (idx == sizeof(SerialFeedback)) {
            // Prüfsumme berechnen
            uint16_t calcChecksum = (uint16_t)(NewFeedback.start ^ 
                                               NewFeedback.cmd1 ^ 
                                               NewFeedback.cmd2 ^ 
                                               NewFeedback.speedR_meas ^ 
                                               NewFeedback.speedL_meas ^ 
                                               NewFeedback.batVoltage ^ 
                                               NewFeedback.boardTemp ^ 
                                               NewFeedback.cmdLed);

            if (calcChecksum == NewFeedback.checksum) {
                if (xSemaphoreTake(feedbackMutex, (TickType_t)5) == pdTRUE) {
                    memcpy(&Feedback, &NewFeedback, sizeof(SerialFeedback));
                    xSemaphoreGive(feedbackMutex);
                }

                // Busy Flag extrahieren
                hoverboardIsBusy = (Feedback.cmdLed & 0x0100) != 0;

                // Daten interpretieren
                if (hoverboardIsBusy) {
                    feedbackDistL = Feedback.cmd1;
                    feedbackDistR = Feedback.cmd2;
                } else {
                    // Nur nullen, wenn keine Mission aktiv ist
                    extern std::atomic<int> currentSkill;
                    if (currentSkill.load() != 20) {
                        feedbackDistL = 0;
                        feedbackDistR = 0;
                    }
                }
            }
            idx = 0;
        }
        incomingBytePrev = incomingByte;
    }
}

// ====================================================================================
// ===== SEND FUNKTION =================================================================
// ====================================================================================
// WURDE AUS main.cpp VERSCHOBEN (To-Do 7: Vollständige Modularisierung)

void Send(int16_t uSteer, int16_t uSpeed) {
    static unsigned long lastHeartbeatSend = 0;
    if (millis() - lastHeartbeatSend < 50) return; 
    lastHeartbeatSend = millis();

    // Safety-Checks aus Globals
    extern bool isHighVoltageShutdown;
    extern std::atomic<bool> skill8SafetyActive;
    
    if (isHighVoltageShutdown || skill8SafetyActive.load()) {
        uSteer = 0; uSpeed = 0;
    }

    // Mutex-Schutz für UART-Kommunikation
    if (xSemaphoreTake(bufferMutex, (TickType_t)10) == pdTRUE) {
        Command.start     = 0xABCD;
        Command.cmdCode   = (int16_t)global_cmdCode; 
        
        Command.steer = uSteer;
        Command.speed = uSpeed;

        Command.maxSpeedL = global_maxSpeedL;
        Command.maxSpeedR = global_maxSpeedR;
        Command.accPct    = (int16_t)global_accel; 
        Command.brkPct    = (int16_t)global_brake;

        // Checksumme berechnen
        Command.checksum = Command.start ^ 
                           Command.cmdCode ^ 
                           Command.steer ^ 
                           Command.speed ^ 
                           Command.maxSpeedL ^ 
                           Command.maxSpeedR ^ 
                           Command.accPct ^ 
                           Command.brkPct;

        // Über UART senden
        HoverSerial->write((uint8_t *)&Command, sizeof(SerialCommand));
        
        xSemaphoreGive(bufferMutex);
    }
}

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
