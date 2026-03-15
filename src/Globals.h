/*
 * Globals.h
 * 
 * Teil 2 (To-Do 7): Globale Variablen als extern deklariert
 * Diese Datei wird von allen Modulen includiert, die auf globale
 * Variablen aus main.cpp zugreifen müssen.
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <atomic>
#include "Config.h"
#include "HoverboardComm.h"

// ====================================================================================
// ===== SKILL & STATE VARIABLEN =======================================================
// ====================================================================================
extern bool skill8Active;
extern bool hoverboardInverted;
extern bool monitorDirection;
extern bool holdLine;
extern int currentCase;
extern int turnDirection;
extern float targetAngle;
extern float startYaw;
extern unsigned long case3StartTime;

// ====================================================================================
// ===== STEUERUNGS-VARIABLEN ==========================================================
// ====================================================================================
extern std::atomic<bool> skillActive;
extern std::atomic<int> currentSkill;
extern int skillSteer;

// ====================================================================================
// ===== INPUT VARIABLEN ===============================================================
// ====================================================================================
extern volatile int16_t input_EspNowSpeed;
extern volatile int16_t input_EspNowSteer;
extern volatile int16_t input_JoySpeed;
extern volatile int16_t input_JoySteer;

// ====================================================================================
// ===== SAFETY VARIABLEN ==============================================================
// ====================================================================================
extern int16_t originalMaxSpeed;
extern bool isHighVoltageShutdown;
extern std::atomic<bool> isInSafetyMode;
extern std::atomic<bool> skill8SafetyActive;

// ====================================================================================
// ===== HOVERBOARD FEEDBACK (aus HoverboardComm.h) ====================================
// ====================================================================================
// Diese sind bereits in HoverboardComm.h als extern deklariert:
// - hoverboardIsBusy
// - feedbackDistL, feedbackDistR

// ====================================================================================
// ===== NETWORK VARIABLEN =============================================================
// ====================================================================================
extern bool isWiFiConnected;
extern bool apShutoffInitiated;
extern unsigned long apStartTime;

// ====================================================================================
// ===== WEBSOCKET & SERVER ============================================================
// ====================================================================================
// Diese sind in Config.h als extern deklariert:
// - server, ws

// ====================================================================================
// ===== LOGGING VARIABLEN =============================================================
// ====================================================================================
extern bool webLogActive;
extern char webLogBuffer[];
extern volatile size_t webLogWriteIndex;
extern volatile size_t webLogReadIndex;
extern volatile bool webLogOverflow;

// ====================================================================================
// ===== FUNKTIONEN (Forward Declarations) =============================================
// ====================================================================================
// Skills
void monitorSkill8Angles(float pitch, float roll);
void monitorDirectionChange(float yaw);
void holdTheLine(float yaw);
void resetHorizon();
void resetAll();

// Blade
void runBladeLogic();

// Safety
void handleSystemStatus(int16_t battery, int16_t temp);

#endif // GLOBALS_H
