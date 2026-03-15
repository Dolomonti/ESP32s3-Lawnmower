/*
 * Config.h
 * 
 * Phase 4: Zentrale Konfigurationsdatei
 * Enthält alle Konstanten, Pin-Definitionen, Enums und die Settings-Struktur
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <atomic>
#include <ESP32Servo.h>
#include <ESPAsyncWebServer.h>

// ====================================================================================
// ===== HARDWARE PINS =================================================================
// ====================================================================================

// Hoverboard UART
#define HOVER_RX_PIN             18          // GPIO for HoverSerial TX (blue cable)
#define HOVER_TX_PIN             17          // GPIO for HoverSerial RX (green cable)

// Onboard LED
#ifndef LED_BUILTIN
  #define LED_BUILTIN            2           // Onboard LED pin (fallback)
#endif

// Buttons
#define BUTTON1_PIN              10          // GPIO for Button 1
#define BUTTON2_PIN              11          // GPIO for Button 2

// Blade Control
#define BLADE_UP_PIN             35          // GPIO for blade lift actuator (up)
#define BLADE_DOWN_PIN           36          // GPIO for blade lift actuator (down)
#define BLADE_ESC_PIN            4           // PWM pin for blade ESC
#define BLADE_RELAY_PIN          5           // Relay for blade safety cutoff
#define BLADE_UNIT_PIN           19          // Power relay for Blade Unit
#define DRIVE_UNIT_PIN           20          // Power relay for Drive Unit
#define BLADE_BATTERY_PIN        1           // ADC pin for blade battery voltage

// Safety
#define EMERGENCY_STOP_PIN       15          // GPIO for emergency stop button

// I2C (MPU6050)
#define MPU_SDA_PIN              13          // I2C SDA
#define MPU_SCL_PIN              14          // I2C SCL
#define MPU_INT_PIN              46          // MPU6050 Interrupt

// ====================================================================================
// ===== KONSTANTEN ====================================================================
// ====================================================================================

// Serial Kommunikation
#define HOVER_SERIAL_BAUD        115200
#define SERIAL_BAUD              115200
#define START_FRAME              0xABCD      // Start frame for UART protocol

// Timing
#define TIME_SEND                100         // Sending interval (ms)
#define HEARTBEAT_INTERVAL       5000        // Heartbeat interval (ms)
#define TIMEOUT_THRESHOLD        15000       // Timeout threshold (ms)
#define SAFETY_TRIGGER_DELAY     5000        // Safety shutdown delay (5 sec)

// Blade ESC
#define BLADE_ZERO_US            1500        // ESC neutral position

// Limits
#define MAX_SPEED                1000        // Maximum allowed speed
#define MAX_STEER                1000        // Maximum allowed steering
#define CENTER_ZONE_THRESHOLD    5.0         // Center threshold for hold the line

// Temperature Thresholds (in 0.01°C)
#define TEMP_SHUTDOWN_DEGREES    8000        // 80.0°C - System shutdown
#define TEMP_CRITICAL_DEGREES    7000        // 70.0°C - Critical warning
#define TEMP_WARNING_DEGREES     6000        // 60.0°C - Warning

// Web Log Buffer
#define WEB_LOG_BUFFER_SIZE      2048        // Size of ring buffer for web logs

// Status Update Interval
#define STATUS_SEND_INTERVAL     2000        // Send status every 2 seconds

// Blade Reset Timing
#define RESET_RAMP_MS            3000        // 3 seconds ramp up
#define RESET_HOLD_MS            1000        // 1 second hold

// ====================================================================================
// ===== EEPROM/NVS ADDRESSES (legacy, kept for reference) =============================
// ====================================================================================

#define SSID_ADDR                0           // EEPROM address for SSID
#define PASSWORD_ADDR            32          // EEPROM address for password
#define SETTINGS_ADDR            100         // EEPROM address for settings

// ====================================================================================
// ===== NVS NAMESPACES ================================================================
// ====================================================================================

#define NVS_NAMESPACE_MOWER      "mower"     // For settings
#define NVS_NAMESPACE_WIFI       "wifi"      // For WiFi credentials

// ====================================================================================
// ===== ANGLE & PD CONTROLLER CONSTANTS ===============================================
// ====================================================================================

constexpr float ANGLE_NORMALIZE_MIN    = -180.0f;
constexpr float ANGLE_NORMALIZE_MAX    = 180.0f;
constexpr float CAPSIZE_THRESHOLD_PITCH = 60.0f;
constexpr float CAPSIZE_THRESHOLD_ROLL  = 60.0f;
constexpr float PD_ERROR_THRESHOLD      = 2.0f;
constexpr float TURN_ANGLE_90           = 90.0f;
constexpr float TURN_ANGLE_180          = 180.0f;

// ====================================================================================
// ===== BLADE CONTROL CONSTANTS =======================================================
// ====================================================================================

constexpr int16_t PWM_WEB_OFFSET        = 20000;    // Base offset for web PWM
constexpr int16_t PWM_WEB_MIN           = 21000;    // Minimum web PWM key
constexpr int16_t PWM_WEB_MAX           = 22000;    // Maximum web PWM key
constexpr int16_t KEY_BLADE_SPEED1      = 25600;    // Activate working speed
constexpr int16_t KEY_BLADE_SPEED1_ALT  = 2;        // Alternative for speed 1
constexpr int16_t KEY_BLADE_SPEED2      = 26500;    // Activate stage 2 speed
constexpr int16_t KEY_BLADE_SPEED2_ALT  = 3;        // Alternative for speed 2
constexpr int16_t BLADE_OFF_CMD         = -1;       // Blade off command
constexpr int16_t BLADE_ON_CMD          = 1;        // Blade on command
constexpr int16_t MIN_RPM_THRESHOLD     = 1000;     // Minimum RPM for cable reset

// Skill config modes
constexpr int16_t CONFIG_MODE_ANGLE     = 1000;     // Set capsize angle
constexpr int16_t CONFIG_MODE_TIMEOUT   = 1001;     // Set capsize timeout

// ====================================================================================
// ===== SKILL SYSTEM ENUMS ============================================================
// ====================================================================================

enum SkillCode : uint8_t {
    SKILL_NONE = 0,
    SKILL_RESET = 1,
    SKILL_TURN_90 = 2,
    SKILL_HOLD_LINE = 3,
    SKILL_TURN_180 = 4,
    SKILL_BLADE_LIFT = 5,
    SKILL_BLADE_CONTROL = 6,
    SKILL_SET_LIMITS = 7,
    SKILL_ESTOP = 8,
    SKILL_REBOOT = 9,
    SKILL_SET_BLADE_SPEEDS = 10,
    SKILL_CABLE_RESET = 11,
    SKILL_DRIVE_LEVELS = 12,
    SKILL_BLADE_LEVELS = 13,
    SKILL_CAPSIZE_PARAMS = 14,
    SKILL_PD_GAINS = 15,
    SKILL_BLADE_BATTERY_FACTOR = 16,
    SKILL_DRIVE_BATTERY_FACTOR = 17,
    SKILL_DRIVE_POWER = 18,
    SKILL_HOVERBOARD_PARAMS = 19,
    SKILL_DISTANCE_MISSION = 20,
    SKILL_HEARTBEAT = 255
};

// ====================================================================================
// ===== BLADE STATE ENUMS =============================================================
// ====================================================================================

enum BladeState {
    BLADE_OFF,
    BLADE_WORK,         // Normal mowing
    BLADE_CABLE_RESET   // Cable reset ramp
};

enum BladeHeight {
    BLADE_HEIGHT_UNKNOWN,
    BLADE_HEIGHT_UP,
    BLADE_HEIGHT_DOWN
};

// ====================================================================================
// ===== SETTINGS STRUCTURE ============================================================
// ====================================================================================

struct Settings {
    uint32_t magic;     // Magic number (0xDEADBEEF) to check initialization

    // Drive System
    int16_t driveMinShutdownVoltage;
    int16_t driveSafetyModeVoltage;
    int16_t driveHighVoltage;
    int16_t driveEmergencyLowTemp;
    int16_t driveSafetyModeTemp;
    int16_t driveHighTemp;

    // Blade System
    int16_t bladeMinShutdownVoltage;
    int16_t bladeSafetyModeVoltage;
    int16_t bladeHighVoltage;
    int16_t bladeEmergencyLowTemp;
    int16_t bladeSafetyModeTemp;
    int16_t bladeHighTemp;

    // Speed & Steering
    int16_t maxSpeed;
    int16_t maxSteer;
    int16_t currentMaxSpeed;
    int16_t currentMaxSteer;

    // Battery Calibration
    float bladeBatteryFactor;
    float driveBatteryFactor;

    // Blade Speeds
    int16_t bladeWorkingSpeed;
    int16_t bladeStage2Speed;
    int16_t bladeMaxSpeed;
    int16_t bladeCableResetRpm;
    int16_t bladeResetRampS;
    int16_t bladeResetDurationS;

    // Capsize Protection
    float capsizeAngle;
    int16_t capsizeTimeout;

    // PD Controller
    float Kp;
    float Kd;

    // API Key
    char apiKey[65];
};

// ====================================================================================
// ===== DEBUG MACROS ==================================================================
// ====================================================================================
// HINWEIS: DEBUG_LOG und DEBUG_PRINTF sind in main.cpp definiert,
// da debugPrintln/debugPrintf dort als Template definiert sind

// ====================================================================================
// ===== EXTERN DECLARATIONS ===========================================================
// ====================================================================================

// Global variables (defined in main.cpp)
extern Settings currentSettings;
extern Servo bladeEsc;
extern BladeState currentBladeState;
extern BladeHeight currentBladeHeight;
extern std::atomic<bool> isInSafetyMode;
extern std::atomic<bool> skill8SafetyActive;
extern std::atomic<bool> skillActive;
extern std::atomic<int> currentSkill;
extern std::atomic<bool> hoverboardIsBusy;
extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern char stored_ssid[32];
extern char stored_password[32];
extern bool isWiFiConnected;

#endif // CONFIG_H
