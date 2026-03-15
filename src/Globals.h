/*
 * Globals.h
 * 
 * Teil 2 (To-Do 7): Globale Variablen als extern deklariert
 * Diese Datei wird von allen Modulen includiert, die auf globale
 * Variablen aus main.cpp zugreifen müssen.
 * 
 * SICHERHEITSNETZ für echte Modularisierung (Option A)
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <atomic>
#include "Config.h"
#include "HoverboardComm.h"
#include "MPU6050_6Axis_MotionApps20.h"

// ====================================================================================
// ===== BLADE CONTROL VARIABLEN =======================================================
// ====================================================================================
extern unsigned long bladeSequenceStartTime;
extern int current_blade_pwm;

// ====================================================================================
// ===== SAFETY VARIABLEN ==============================================================
// ====================================================================================
extern int16_t originalMaxSpeed;
extern bool isHighVoltageShutdown;
extern std::atomic<bool> isInSafetyMode;
extern std::atomic<bool> skill8SafetyActive;
extern std::atomic<bool> skillActive;
extern std::atomic<int> currentSkill;
// hoverboardIsBusy ist bereits in HoverboardComm.h

// ====================================================================================
// ===== TIMER VARIABLEN ===============================================================
// ====================================================================================
extern unsigned long lastStatusSend;
extern const long statusSendInterval;

// ====================================================================================
// ===== NETWORK VARIABLEN =============================================================
// ====================================================================================
extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern char stored_ssid[32];
extern char stored_password[32];
extern bool isWiFiConnected;
extern bool apShutoffInitiated;
extern unsigned long apStartTime;
extern uint8_t selfMacAddress[6];
extern volatile bool shouldRestart;

// ====================================================================================
// ===== WEB LOG VARIABLEN =============================================================
// ====================================================================================
extern bool webLogActive;
extern char webLogBuffer[];
extern volatile size_t webLogWriteIndex;
extern volatile size_t webLogReadIndex;
extern volatile bool webLogOverflow;
extern unsigned long lastWebLogSend;
extern portMUX_TYPE logMutex;

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
extern int skillSteer;

// ====================================================================================
// ===== INPUT VARIABLEN ===============================================================
// ====================================================================================
extern volatile int16_t input_EspNowSpeed;
extern volatile int16_t input_EspNowSteer;
extern volatile int16_t input_JoySpeed;
extern volatile int16_t input_JoySteer;

// ====================================================================================
// ===== MPU/GYRO VARIABLEN ============================================================
// ====================================================================================
extern MPU6050 mpu;
extern bool dmpReady;
extern uint8_t devStatus;
extern uint16_t packetSize;
extern uint16_t fifoCount;
extern uint8_t fifoBuffer[64];
extern SemaphoreHandle_t mpuSemaphore;
extern SemaphoreHandle_t i2cMutex;
extern float ypr[3];
extern float yaw;
extern float yawOffset;
extern float pitch;
extern float pitchOffset;
extern float roll;
extern float rollOffset;

// ====================================================================================
// ===== FUNKTIONS-CALLBACKS (Forward Declarations) ====================================
// ====================================================================================
// Diese Funktionen sind in main.cpp definiert, werden aber von Modulen aufgerufen

// Skill System
void triggerSkill(uint8_t skill, int16_t param1 = 0, int16_t param2 = 0, int16_t param3 = 0, int16_t param4 = 0, int16_t param5 = 0, int16_t param6 = 0);
void monitorSkill8Angles(float pitch, float roll);
void monitorDirectionChange(float yaw);
void holdTheLine(float yaw);
void resetHorizon();
void resetAll();

// Blade Control
void runBladeLogic();

// System
void handleSystemStatus(int16_t battery, int16_t temp);
void logToWebpage(const char* message);

// WiFi/Network
void readWiFiCredentialsFromNVS();
void writeWiFiCredentialsToNVS(const char *ssid, const char *password);

// Hoverboard (werden nach HoverboardComm.cpp verschoben)
void Send(int16_t uSteer, int16_t uSpeed);
void Receive();

// Web Interface
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void resetDevice(AsyncWebServerRequest *request);
void handleResetWifi(AsyncWebServerRequest *request);
void handleWebpage(AsyncWebServerRequest *request);

#endif // GLOBALS_H
