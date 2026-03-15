/* Lawnmower Project - main.cpp */

#include <Arduino.h>

/*
// ====================================================================================
// ===== KEEP ME IN EVERY VERSION OF THIS CODE HERE ON TOP ============================
// ====================================================================================
//
// This code is intended for the Arduino IDE to program an ESP32 S3 board.
// To ensure successful compilation, please use the following environment setup:
// Partition Scheme ändern: Wähle in der Arduino IDE "Huge APP (3MB No OTA/1MB SPIFFS)". Dies gibt deinem Code mehr als doppelt so viel Platz (3,0 MB statt 1,2 MB).
// -- BOARD MANAGER --
// 1. In Arduino IDE -> Settings -> "Additional boards manager URLs", add:
//    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
// 2. Go to Tools -> Board -> Boards Manager...
// 3. Search for "esp32" and install "esp32 by Espressif Systems" version 3.3.0.
//
// -- REQUIRED LIBRARIES (from your installed list) --
// Use the Arduino Library Manager (Sketch -> Include Library -> Manage Libraries...) to install:
// 1. ArduinoJson by Benoit Blanchon (Version 7.4.2)
// 2. Async TCP by ESP32Async (Version 3.4.7)
// 3. ESP Async WebServer by ESP32Async (Version 3.8.0)
// 4. MPU6050_tockn by tockn (Version 1.5.2) - This is likely the MPU6050 library being used.
//ESP32 Project Library List:

Here is a complete list of the libraries installed in your Arduino IDE, based on the provided screenshots.

Core Libraries for This Project
ArduinoJson by Benoit Blanchon (Version 7.4.2)

Async TCP by ESP32Async (Version 3.4.7)

ESP Async WebServer by ESP32Async (Version 3.8.0)

MPU6050_tockn by tockn (Version 1.5.2)

Other Installed Libraries
NTPClient by Fabrice Weinberg (Version 3.2.1)

Servo by Michael Margolis, Arduino (Version 1.2.2)

ArduinoSTL by Mike Matera (Version 1.3.3)

BluetoothSerial by Henry Abrahamsen (Version 1.1.0)

DoubleResetDetector_C by Khoi Hoang (Version 1.8.1)

ESP32Servo by Kevin Harrington, John K... (Version 3.0.8)

ESP_AT_Lib by Khoi Hoang (Version 1.5.1)

EthernetWebServer by Khoi Hoang (Version 2.4.1)

EthernetWebServer_STM by Khoi Hoang (Version 1.5.0)

Ethernet_Generic by Various (Version 2.8.1)

FlashStorage_RTL8720 by Khoi Hoang (Version 1.1.0)

FlashStorage_SAMD by Cristian Maglie (Version 1.3.2)

FlashStorage_STM32 by Khoi Hoang (Version 1.2.0)

FlashStorage_STM32F1 by Khoi Hoang (Version 1.1.0)

Functional-Vlpp by Khoi Hoang (Version 1.0.2)

NimBLE-Arduino by h2zero (Version 2.3.4)

PS3 Controller Host by Jeffrey van Pernis (Version 2.0.0)

STM32duino LwIP by Adam Dunkels... (Version 2.1.3)

STM32duino STM32Ethernet by... (Version 1.4.0)

SinricPro_Generic by Boris Jaeger... (Version 2.8.5)

UIPEthernet by Norbert Truchsess, Cassy... (Version 2.0.12)

USB Host Shield Library 2.0 by Oleg... (Version 1.7.0)

WebServer_WT32_ETH0 by Khoi Hoang (Version 1.5.1)

WebSockets_Generic by Markus Sattler, Khoi... (Version 2.16.1)

WiFi101_Generic by Arduino (Version 1.0.0)

WiFiEspAT by Juraj Andrassy (Version 1.5.0)

WiFiMulti_Generic by Khoi Hoang (Version 1.2.2)

WiFiNINA_Generic by Arduino, Khoi Hoang... (Version 1.8.15-1)

WiFiWebServer by Khoi Hoang (Version 1.10.1)
// ====================================================================================
*/

#define ENABLE_DEBUG_SERIAL true // Set to false to disable most Serial output

#define PRINT_FEEDBACK_BLOCK false  // Hoverboard RX print out in Serial Monitor

#define HOVER_SERIAL_BAUD        115200      // Baud rate for HoverSerial (hoverboard communication)
#define SERIAL_BAUD              115200      // Baud rate for built-in Serial (Serial Monitor)
#define START_FRAME              0xABCD      // Start frame for reliable serial communication
#define TIME_SEND                100         // Sending time interval (ms)
#define HEARTBEAT_INTERVAL       5000        // Heartbeat interval (ms)
#define TIMEOUT_THRESHOLD        15000       // Timeout threshold (ms)


#define CENTER_ZONE_THRESHOLD 5.0    // Programmable center threshold for Case 3, hold the line mode
// #define SKILL8_THRESHOLD_ANGLE 45.0 // <<< REMOVED: This is now a configurable setting in the Settings struct

#define MAX_SPEED 1000    // Maximum allowed speed
#define MAX_STEER 1000    // Maximum allowed steering

// These are now part of the Settings struct, but we keep them here as initial defaults
// if the EEPROM is empty.
// int16_t currentMaxSpeed = 200;  // Dynamic maximum speed
// int16_t currentMaxSteer = 200;  // Dynamic maximum steering
// int16_t bladeWorkingSpeed = 0;
// int16_t bladeStage2Speed = 0;
// int16_t bladeMaxSpeed = 9000;


#define HOVER_RX_PIN             18          // GPIO for HoverSerial TX blue cable
#define HOVER_TX_PIN             17          // GPIO for HoverSerial RX green cable
#ifndef LED_BUILTIN
  #define LED_BUILTIN            2           // Onboard LED pin (fallback if not defined by board)
#endif
#define BUTTON1_PIN              10          // GPIO for Button 1
#define BUTTON2_PIN              11          // GPIO for Button 2
#define BLADE_UP_PIN               35          // Example GPIO pin for blade up
#define BLADE_DOWN_PIN           36          // Example GPIO pin for blade down
#define EMERGENCY_STOP_PIN       15            // GPIO pin for the emergency stop button
#define BLADE_BATTERY_PIN            1         // to measure the minus pole of the 4S battery and have multiple resistors inbetween!
#define BLADE_UNIT_PIN               19        // Power Pin für Blade Unit (GPIO 19)
#define DRIVE_UNIT_PIN               20        // Power Pin für Drive Unit (GPIO 20)


#include <ESP32Servo.h>
const int BLADE_ESC_PIN = 4; // Wunschgemäß auf Pin 4 gesetzt
const int BLADE_RELAY_PIN = 5;
const int BLADE_ZERO_US = 1500; // je nach ESC Voreinstellung

#define SAFETY_TRIGGER_DELAY 5000    // Verzögerung für Sicherheitsabschaltungen Volt un Temperatur in ms (5 Sek)

// ====================================================================================
// ===== PHASE 3 REFACTORING: Constants & Macros ======================================
// ====================================================================================

// --- Angle Constants ---
constexpr float ANGLE_NORMALIZE_MIN = -180.0f;
constexpr float ANGLE_NORMALIZE_MAX = 180.0f;
constexpr float CAPSIZE_THRESHOLD_PITCH = 60.0f;  // Capsize detection angle
constexpr float CAPSIZE_THRESHOLD_ROLL = 60.0f;

// --- PD-Controller Constants ---
constexpr float PD_ERROR_THRESHOLD = 2.0f;  // Target reached threshold in degrees

// --- Skill System Constants (Phase 2 Refactoring) ---
// Turn angles for automated movements
constexpr float TURN_ANGLE_90 = 90.0f;
constexpr float TURN_ANGLE_180 = 180.0f;

// Blade control key codes (Skill 6)
constexpr int16_t PWM_WEB_OFFSET = 20000;      // Base offset for web PWM values
constexpr int16_t PWM_WEB_MIN = 21000;         // Minimum web PWM key
constexpr int16_t PWM_WEB_MAX = 22000;         // Maximum web PWM key
constexpr int16_t KEY_BLADE_SPEED1 = 25600;    // Activate working speed
constexpr int16_t KEY_BLADE_SPEED1_ALT = 2;    // Alternative for speed 1
constexpr int16_t KEY_BLADE_SPEED2 = 26500;    // Activate stage 2 speed
constexpr int16_t KEY_BLADE_SPEED2_ALT = 3;    // Alternative for speed 2

// Skill 8 config modes
constexpr int16_t CONFIG_MODE_ANGLE = 1000;    // Set capsize angle
constexpr int16_t CONFIG_MODE_TIMEOUT = 1001;  // Set capsize timeout

// Skill 11 minimum RPM threshold
constexpr int16_t MIN_RPM_THRESHOLD = 1000;    // Minimum RPM for cable reset

// --- Skill System Enums (Phase 4 Refactoring) ---
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

// Blade control sub-commands
constexpr int16_t BLADE_OFF_CMD = -1;
constexpr int16_t BLADE_ON_CMD = 1;

// --- Debug Logging Macro (Phase 3.2) ---
// Usage: DEBUG_LOG("Message"); or DEBUG_PRINTF("Value: %d\n", val);
#define DEBUG_LOG(msg) do { if (ENABLE_DEBUG_SERIAL) debugPrintln(msg); } while(0)
#define DEBUG_PRINTF(fmt, ...) do { if (ENABLE_DEBUG_SERIAL) debugPrintf(fmt, ##__VA_ARGS__); } while(0)

// ====================================================================================

// 1. ESP-NOW Command Struktur
typedef struct __attribute__((packed)) {
    uint16_t start;       // Start frame
    int16_t steer;
    int16_t speed;
    uint8_t commandCode;  // Skill command code
    uint16_t checksum;    // Checksum
} ESPNowCommand;

SemaphoreHandle_t espNowMutex;

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

// WICHTIG: Command wird hier EINMAL deklariert
SerialCommand Command;

// Globale Variablen für die Steuerung
uint16_t global_cmdCode = 0;   
uint16_t global_accel = 10;    
uint16_t global_brake = 100;   
int16_t global_maxSpeedL = 0; // FIX: Deklaration hinzugefügt
int16_t global_maxSpeedR = 0; // FIX: Deklaration hinzugefügt

// pinMode(BLADE_UP_PIN, OUTPUT);
// pinMode(BLADE_DOWN_PIN, OUTPUT);

#include <WiFi.h>
#include <DNSServer.h>
#include <esp_now.h>
#include <HardwareSerial.h>
#include <esp_wifi.h>

#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>

#include <ArduinoJson.h>

#include <EEPROM.h>
#include <ArduinoOTA.h>
#include "esp_crc.h" // Dies für die CRC32-Berechnung hinzufügen

DNSServer dnsServer;        // <--- DNS Server Instanz
const byte DNS_PORT = 53;

#define ESPNOW_FIXED_CHANNEL 6

#define EEPROM_SIZE 512 // Adjust based on your requirements
#define SSID_ADDR 0      // EEPROM address for SSID
#define PASSWORD_ADDR 32 // EEPROM address for password
#define SETTINGS_ADDR 100 // EEPROM address for settings structure

// --- Settings Structure for EEPROM Persistence ---
struct Settings {
    uint32_t magic; // Magic number to check if settings are initialized

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

    float bladeBatteryFactor; // Der Multiplikator für blade batterz adjustment (z.B. 23.0)
    float driveBatteryFactor;

    // Blade Speeds
    int16_t bladeWorkingSpeed;
    int16_t bladeStage2Speed;
    int16_t bladeMaxSpeed;
    int16_t bladeCableResetRpm;
    int16_t bladeResetRampS;      
    int16_t bladeResetDurationS;

    float capsizeAngle;     // The angle (in degrees) at which capsize is detected.
    int16_t capsizeTimeout; // The time (in ms) the angle must be exceeded to trigger a stop.
    
    float Kp; // Proportional Gain
    float Kd; // Derivative Gain

    char apiKey[65]; // Google Gemini API Key
};

Settings currentSettings; // Global settings object

// ---  Blade Control Variablen & State Machine ---
Servo bladeEsc; // Das Objekt zur Motorsteuerung

enum BladeState {
  BLADE_OFF,
  BLADE_WORK,         // Normales Mähen
  BLADE_CABLE_RESET   // Die Rampe für den Kabel-Reset
};

BladeState currentBladeState = BLADE_OFF;

// Blade height state for safety: only allow blade ON / speeds when blade is DOWN
enum BladeHeight {
  BLADE_HEIGHT_UNKNOWN,
  BLADE_HEIGHT_UP,
  BLADE_HEIGHT_DOWN
};
BladeHeight currentBladeHeight = BLADE_HEIGHT_UNKNOWN;

unsigned long bladeSequenceStartTime = 0;
int current_blade_pwm = BLADE_ZERO_US; 

// Konstanten für den Reset (festgelegt wie im alten Code, da nicht im EEPROM struct vorhanden)
const unsigned long RESET_RAMP_MS = 3000; // 3 Sekunden Hochfahren
const unsigned long RESET_HOLD_MS = 1000; // 1 Sekunde Halten

// State variables for the new safety logic
bool isInSafetyMode = false;
int16_t originalMaxSpeed = 0; // Stores max speed before entering safety mode
bool isHighVoltageShutdown = false;

// Temperature monitoring thresholds (unchanged)
#define TEMP_SHUTDOWN_DEGREES        8000  // System shuts down above 80.0 C
#define TEMP_CRITICAL_DEGREES        7000  // Critical warning above 70.0 C
#define TEMP_WARNING_DEGREES         6000  // Warning above 60.0 C

// Timer for sending status updates to the webpage
unsigned long lastStatusSend = 0;
const long statusSendInterval = 2000; // Send status every 2 seconds


AsyncWebServer server(80); // Define the web server
AsyncWebSocket ws("/ws");  // Define the WebSocket path
char stored_ssid[32] = ""; // Store WiFi SSID
char stored_password[32] = ""; // Store WiFi password
bool isWiFiConnected = false; // WiFi connection status
bool apShutoffInitiated = false; // AP shutdown status
unsigned long apStartTime = 0; // AP start time
uint8_t selfMacAddress[6]; // Declare the MAC address variable
volatile bool shouldRestart = false; // Flag to trigger restart from main loop


// ====================================================================================
// ===== NEU: Web Serial Puffer Variablen & Funktionen ================================
// ====================================================================================
bool webLogActive = false;       // Schalter: Soll ans Web gesendet werden?
String webLogBuffer = "";        // Der Puffer, der Text sammelt
unsigned long lastWebLogSend = 0;// Timer für das 0.5s Intervall

// 1. Ersatz für debugPrint / debugPrintln
template <typename T>
void debugPrint(T msg) {
    Serial.print(msg); // Immer an USB senden
    if (webLogActive) {
        webLogBuffer += String(msg);
        if (webLogBuffer.length() > 2000) webLogBuffer = "--- BUFFER OVERFLOW ---\n"; 
    }
}

template <typename T>
void debugPrintln(T msg) {
    Serial.println(msg); // Immer an USB senden
    if (webLogActive) {
        webLogBuffer += String(msg) + "\n"; 
         if (webLogBuffer.length() > 2000) webLogBuffer = "--- BUFFER OVERFLOW ---\n";
    }
}

// Overloads for IPAddress to avoid ambiguous conversions to String
void debugPrint(const IPAddress &msg) {
    Serial.print(msg);
    if (webLogActive) {
        webLogBuffer += msg.toString();
        if (webLogBuffer.length() > 2000) webLogBuffer = "--- BUFFER OVERFLOW ---\n";
    }
}

void debugPrintln(const IPAddress &msg) {
    Serial.println(msg);
    if (webLogActive) {
        webLogBuffer += msg.toString() + "\n";
        if (webLogBuffer.length() > 2000) webLogBuffer = "--- BUFFER OVERFLOW ---\n";
    }
}

// *** NEU: Overloads für float um String-Casting-Fehler zu vermeiden ***
void debugPrint(float msg) {
    Serial.print(msg, 2); // 2 Nachkommastellen
    if (webLogActive) {
        webLogBuffer += String(msg, 2);
        if (webLogBuffer.length() > 2000) webLogBuffer = "--- BUFFER OVERFLOW ---\n";
    }
}

void debugPrintln(float msg) {
    Serial.println(msg, 2);
    if (webLogActive) {
        webLogBuffer += String(msg, 2) + "\n";
        if (webLogBuffer.length() > 2000) webLogBuffer = "--- BUFFER OVERFLOW ---\n";
    }
}

// 2. Ersatz für debugPrintf
void debugPrintf(const char *format, ...) {
    char loc_buf[128]; 
    va_list args;
    va_start(args, format);
    vsnprintf(loc_buf, sizeof(loc_buf), format, args);
    va_end(args);

    Serial.print(loc_buf); // Immer an USB
    if (webLogActive) {
        webLogBuffer += String(loc_buf);
        if (webLogBuffer.length() > 2000) webLogBuffer = "--- BUFFER OVERFLOW ---\n";
    }
}
// ====================================================================================

// ########################## DEFAULT SKILL ACTIVATION at start up ##########################

/*
    Configure the default active state of each skill here.
    Set to 'true' to have the skill active by default, or 'false' otherwise.
    Modify these values to change which skills are active on startup.
*/

const bool DEFAULT_SKILL1_ACTIVE = false;
const bool DEFAULT_SKILL2_ACTIVE = false;
const bool DEFAULT_SKILL3_ACTIVE = false;
const bool DEFAULT_SKILL4_ACTIVE = false;
const bool DEFAULT_SKILL5_ACTIVE = false;
const bool DEFAULT_SKILL6_ACTIVE = false;
const bool DEFAULT_SKILL7_ACTIVE = false;
const bool DEFAULT_SKILL8_ACTIVE = true; // Skill 8 active by default, Capsizing     ALL OFF
const bool DEFAULT_SKILL9_ACTIVE = false;

// ########################## END DEFAULT SKILL ACTIVATION ##########################


HardwareSerial HoverSerial(1);        // Use UART1 for hoverboard communication

// Global variables
uint8_t idx = 0;
uint16_t bufStartFrame;
byte *p;  // Pointer for serial data parsing in Receive()
byte incomingByte;
byte incomingBytePrev;




unsigned long lastHeartbeat = 0;

///////////////////////MAC ADRESS INSERT HERE//////////////////////////

struct Device {
    uint8_t mac[6]; // MAC address of the device
    int id;         // ID associated with the device
};

Device deviceList[] = {
    {{0x48, 0x27, 0xE2, 0xE9, 0x11, 0x9C}, 1},
    {{0xFC, 0xB4, 0x67, 0x73, 0xF1, 0x10}, 2},
    {{0xDC, 0xDA, 0x0C, 0x52, 0xDE, 0x84}, 3},
    {{0x30, 0xC6, 0xF7, 0x25, 0xFD, 0xF8}, 4},
    //add new Mac Address devices here
};

int deviceID = -1;  // Variable to store the assigned device ID

const int numberOfPeers = sizeof(deviceList) / sizeof(deviceList[0]); // Define the number of peers

// NEW: Struct to hold all peer-related information
struct PeerInfo {
    bool isConnected;
    unsigned long lastSeen;
    int reconnectAttempts;
    unsigned long lastReconnectAttempt;
};
PeerInfo peers[numberOfPeers]; // Array to track each peer's state


///////////////////////////////////////////////////////////////////


// Function prototypes
bool saveSettings();
void loadSettings();
void monitorDirectionChange(float yaw);
void holdTheLine(float yaw);
void resetHorizon();
void resetAll();
void core1WiFiTask(void * parameter);
void Send(int16_t uSteer, int16_t uSpeed);
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void resetDevice(AsyncWebServerRequest *request);
void handleResetWifi(AsyncWebServerRequest *request);
extern const char webpage[] PROGMEM; // Correct declaration for an array in program memory
void handleSystemStatus(int16_t battery, int16_t temp);
void handleWebpage(AsyncWebServerRequest *request);
void connectToWiFi(const char* ssid, const char* password);
void logToWebpage(const String& message);
void espNowTask(void *pvParameters);
void mpuReadTask(void *parameter);
void statusTask(void *pvParameters);
void controlLogicTask(void *pvParameters); // Neue Zeile hinzugefügt
void eepromSaveTask(void *pvParameters); // Task 5.5: EEPROM Speicher-Task

//Gyro Initialization

MPU6050 mpu;

bool dmpReady = false;  // Set true if DMP is initialized successfully
uint8_t devStatus;      // Return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // Expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // Count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
SemaphoreHandle_t mpuSemaphore = NULL; // Semaphore to signal MPU data is ready
float lastError = 0.0f;
unsigned long lastPdTime = 0;

Quaternion q;           // [w, x, y, z] quaternion container
VectorFloat gravity;    // [x, y, z] gravity vector
float ypr[3];           // [yaw, pitch, roll] container and gravity vector

// Global yaw, pitch, and roll offsets
float yawOffset = 0.0;
float pitchOffset = 0.0;
float rollOffset = 0.0;

// Global variables for yaw, pitch, roll
float yaw = 0.0, pitch = 0.0, roll = 0.0;

// Case-related variables

bool skill8Active = false;        // State of Skill 8
bool skill8SafetyActive = false;      // Safety signal active state (sending stop commands)
bool hoverboardInverted = false;      // To detect capsizing

int currentCase = 0;          // Case trigger variable (1, 2, 3, 4, or 5)
bool monitorDirection = false;      // Flag for Cases 2 and 4
bool holdLine = false;              // Flag for Case 3
float targetAngle = 0;              // Target angle for Cases 2 and 4
int turnDirection = 0;              // direction achange awareness
float startYaw = 0;                 // Initial yaw for Cases 3 and 5
unsigned long case3StartTime = 0;   // Timer for Case 3 delay
unsigned long lastSerialUpdate = 0;       // Timer for serial updates


unsigned long capsizeDetectTime = 0;

// Task 5.5: Flag für asynchrones EEPROM-Speichern
volatile bool settingsNeedSave = false; // 


 

// Hoverboard Uart Serial output structure (perhaps only with TTL adapter; not working atm)
// WICHTIG: __attribute__((packed)) hinzugefügt, um exakt 18 Bytes zu garantieren
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
SerialFeedback Feedback;
SerialFeedback NewFeedback;


bool skillActive = false;       // True if skill 2,3,4 is active
int currentSkill = 0;           // Which skill is active (2, 3, or 4)
int skillSteer = 0;             // Steering override generated by skill logic
// --- Input-Quellen klar getrennt (Refactoring Task 2.2) ---
volatile int16_t input_EspNowSpeed = 0;   // war: input_EspNowSpeed
volatile int16_t input_EspNowSteer = 0;   // war: input_EspNowSteer
volatile int16_t input_JoySpeed = 0;      // war: input_JoySpeed
volatile int16_t input_JoySteer = 0;      // war: input_JoySteer 
volatile bool hoverboardIsBusy = false; 
volatile int16_t feedbackDistL = 0;
volatile int16_t feedbackDistR = 0;       // FIX: Fehlte in deiner Liste
volatile unsigned long skill20StartTime = 0; // FIX: Fehlte in deiner Liste
volatile bool resetMissionFlags = false;    // FIX: Fehlte in deiner Liste
// --------------------------------------------------------------------------------


unsigned long lastJoystickCommandTime = 0;


// Timeout for Peer Connection
const unsigned long peerTimeout = 15000; // Timeout duration for each peer (15 seconds)
const unsigned long reconnectInterval = 2000; // Time between reconnect attempts
const int maxReconnectAttempts = 1; // Max reconnect attempts

volatile int16_t corrected_batVoltage = 0; // Stores the corrected voltage for the web UI
volatile int32_t global_blade_voltage_mv = 0; // Speichert den Wert in Millivolt (z.B. 24000 für 24V)
volatile float current_speed_kmh = 0.0; // Stores the calculated speed in km/h

//Buffer Synchronization with Mutex: Synchronize shared buffers using a mutex to avoid data corruption
SemaphoreHandle_t bufferMutex = xSemaphoreCreateMutex();
SemaphoreHandle_t feedbackMutex;
// In der Nähe Ihrer anderen Mutex-Deklarationen
SemaphoreHandle_t i2cMutex;



// Example: a function to clear skill states
// *** TASK 3.1: Funktion checkWebSocketConnection() entfernt, direkt ws.cleanupClients() verwenden ***
void readWiFiCredentialsFromEEPROM() {
    for (int i = 0; i < 31; i++) {  // Only read 31 chars to leave room for null terminator
        stored_ssid[i] = char(EEPROM.read(SSID_ADDR + i));
        stored_password[i] = char(EEPROM.read(PASSWORD_ADDR + i));
    }
    // Explicitly null-terminate both strings
    stored_ssid[31] = '\0';
    stored_password[31] = '\0';
}


void writeWiFiCredentialsToEEPROM(const char *ssid, const char *password) {
    // Schreiben Sie die SSID- und Passwort-Zeichenfolgen in den EEPROM.
    // Sicherheitsfix: Länge validieren und null-terminieren
    size_t ssidLen = strlen(ssid);
    size_t passwordLen = strlen(password);

    // Warnung wenn zu lang
    if (ssidLen >= 32 || passwordLen >= 32) {
        DEBUG_LOG("WARNING: WiFi credentials truncated (max 31 chars)");
    }

    // Schreiben mit Null-Terminierung
    for (int i = 0; i < 32; i++) {
        EEPROM.write(SSID_ADDR + i, i < ssidLen ? ssid[i] : 0);
        EEPROM.write(PASSWORD_ADDR + i, i < passwordLen ? password[i] : 0);
    }

    // Übertragen Sie die Änderungen in den permanenten Speicher und überprüfen Sie den Erfolg.
    if (EEPROM.commit()) {
        DEBUG_LOG("WiFi credentials successfully saved to EEPROM.");
        logToWebpage("WiFi credentials saved."); // Informiert auch die Webseite
    } else {
        DEBUG_LOG("ERROR: Failed to save WiFi credentials to EEPROM.");
        logToWebpage("Error: Failed to save WiFi credentials."); // Informiert die Webseite über den Fehler
    }
}

// ESP-NOW Send Callback (required but unused - no action needed)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // Callback is registered but no action required
    (void)mac_addr; (void)status; // Suppress unused parameter warnings
}


// REVISED PEER MANAGEMENT LOGIC
void managePeers() {
    unsigned long currentMillis = millis();

    if (xSemaphoreTake(espNowMutex, (TickType_t)50) != pdTRUE) {
        return; // Could not get mutex, try again later
    }

    for (int i = 0; i < numberOfPeers; i++) {
        // >>> FIX 1: Skip trying to manage our own MAC address
        if (deviceList[i].id == deviceID) {
            continue;
        }

        // 1. Check for timeout if currently connected
        if (peers[i].isConnected && (currentMillis - peers[i].lastSeen > peerTimeout)) {
            peers[i].isConnected = false;
            peers[i].reconnectAttempts = 0; // Reset attempts on disconnect
            DEBUG_PRINTF("Peer %d timed out.\n", deviceList[i].id);
        }

        // 2. Attempt to reconnect (Status-Update ohne Re-Init)
        if (!peers[i].isConnected) {
            if (currentMillis - peers[i].lastReconnectAttempt > reconnectInterval) {
                peers[i].lastReconnectAttempt = currentMillis;
                // Wir löschen den Peer NICHT mehr. ESP-NOW merkt von selbst,
                // wenn das Gerät wieder in Reichweite ist und sendet.
                if (peers[i].reconnectAttempts == 0) {
                    DEBUG_PRINTF("Peer %d marked as OFFLINE. Waiting for signal...\n", deviceList[i].id);
                    peers[i].reconnectAttempts = 1; // Nur einmal loggen
                }
            }
        }
    }
    xSemaphoreGive(espNowMutex);
}


/**
 * @brief Saves the current settings structure AND a CRC32 checksum to EEPROM.
 */
bool saveSettings() {
    // 1. Berechne den CRC32 der Einstellungs-Struktur
    uint32_t calculated_crc = esp_crc32_le(0, (uint8_t*)&currentSettings, sizeof(Settings));

    // 2. Schreibe die Struktur an ihre Adresse
    EEPROM.put(SETTINGS_ADDR, currentSettings);
    
    // 3. Schreibe den berechneten CRC direkt HINTER die Struktur
    EEPROM.put(SETTINGS_ADDR + sizeof(Settings), calculated_crc);

    // 4. Änderungen bestätigen
    if (EEPROM.commit()) {
        DEBUG_LOG("Settings and CRC successfully saved to EEPROM.");
        return true;
    } else {
        DEBUG_LOG("ERROR: Failed to save settings to EEPROM!");
        return false;
    }
}
/**
 * @brief Loads settings from EEPROM or initializes with defaults if CRC check fails.
 */

void loadSettings() {
    // 1. Lade die Einstellungs-Struktur
    EEPROM.get(SETTINGS_ADDR, currentSettings);

    // 2. Lade den gespeicherten CRC (der direkt dahinter liegt)
    uint32_t saved_crc;
    EEPROM.get(SETTINGS_ADDR + sizeof(Settings), saved_crc);

    // 3. Berechne den CRC der Struktur, die wir gerade geladen haben
    uint32_t calculated_crc = esp_crc32_le(0, (uint8_t*)&currentSettings, sizeof(Settings));

    // 4. Vergleiche den gespeicherten CRC mit dem neu berechneten CRC.
    if (calculated_crc != saved_crc || currentSettings.magic != 0xDEADBEEF) {

        DEBUG_LOG("EEPROM data invalid or CRC mismatch! Loading defaults.");

        // *** SICHERHEIT: Struct zurücksetzen vor Default-Zuweisung ***
        memset(&currentSettings, 0, sizeof(Settings));

        // --- Set Default Values --- 
        currentSettings.magic = 0xDEADBEEF; 

        // Drive System Defaults
        currentSettings.driveBatteryFactor = 1.0; // Korrekturwert für 37.0V
        currentSettings.driveMinShutdownVoltage = 3100;
        currentSettings.driveSafetyModeVoltage = 3250;
        currentSettings.driveHighVoltage = 4300;
        currentSettings.driveEmergencyLowTemp = 200;
        currentSettings.driveSafetyModeTemp = 1000;
        currentSettings.driveHighTemp = 5000;

        // Blade System Defaults
        currentSettings.bladeBatteryFactor = 6.40; // Korrekturwert für 14.84V
        currentSettings.bladeMinShutdownVoltage = 1200;
        currentSettings.bladeSafetyModeVoltage = 1300;
        currentSettings.bladeHighVoltage = 1680;
        currentSettings.bladeEmergencyLowTemp = 500;
        currentSettings.bladeSafetyModeTemp = 1000;
        currentSettings.bladeHighTemp = 5000;

        // Speed & Steer Defaults
        currentSettings.maxSpeed = MAX_SPEED;
        currentSettings.maxSteer = MAX_STEER;
        currentSettings.currentMaxSpeed = 200;
        currentSettings.currentMaxSteer = 200;

        // Blade Speeds & Reset Timing
        currentSettings.bladeWorkingSpeed = 1600;
        currentSettings.bladeStage2Speed = 1700;
        currentSettings.bladeMaxSpeed = 1800;
        currentSettings.bladeCableResetRpm = 1800;
        currentSettings.bladeResetRampS = 3;      
        currentSettings.bladeResetDurationS = 1;  

        // Capsize Settings
        currentSettings.capsizeAngle = 60.0;
        currentSettings.capsizeTimeout = 1500;

        // PD-Gains
        currentSettings.Kp = 4.0;
        currentSettings.Kd = 1.5;

        currentSettings.apiKey[0] = '\0'; // Init API Key empty

        // Speichere die neuen Standardwerte
        if (!saveSettings()) {
            DEBUG_LOG("ERROR: Failed to save default settings to EEPROM!");
        }
    } else {
        DEBUG_LOG("Successfully loaded settings from EEPROM (CRC OK).");
    }
}


/////////////////////////TRIGGER SKILLS Logic//////////////////////////////////////////////

/////////////////////////TRIGGER SKILLS Logic//////////////////////////////////////////////

// ---------------------------------------------------------------------
// Forward Declarations (für normalizeAngle in Skill 2, 4)
// ---------------------------------------------------------------------
inline float normalizeAngle(float angle);

// ---------------------------------------------------------------------
// Helper Functions (Phase 4b Refactoring)
// ---------------------------------------------------------------------

/**
 * @brief Stoppt das Messer sicher und protokolliert den Grund
 * @param reason Grund für das Stoppen (für Debug-Log)
 */
inline void stopBladeSafely(const char* reason) {
    currentBladeState = BLADE_OFF;
    digitalWrite(BLADE_UNIT_PIN, LOW);
    current_blade_pwm = BLADE_ZERO_US;
    DEBUG_PRINTF("Blade stopped: %s\n", reason);
}

/**
 * @brief Prüft ob das Messer aktiviert werden darf
 * @return true wenn Messer unten (BLADE_HEIGHT_DOWN), sonst false
 */
inline bool bladeAllowed() {
    return (currentBladeHeight == BLADE_HEIGHT_DOWN);
}

// ---------------------------------------------------------------------
// Sub-Handler 1: Drive & Movement Skills (0, 1, 2, 3, 4, 20)
// ---------------------------------------------------------------------
void handleDriveSkills(uint8_t skill, int16_t param1, int16_t param2, int16_t param3, int16_t param4, int16_t param5, int16_t param6) {
    switch (skill) {
        case SKILL_NONE:
            DEBUG_LOG("Skill 0: undefined placeholder.");
            break;
        case SKILL_RESET:
            DEBUG_LOG("Skill 1: Full System Stop & Reset Triggered.");
            resetAll();
            resetHorizon();
            if (skill8SafetyActive) {
                skill8SafetyActive = false;
                DEBUG_LOG("--> Emergency Stop cleared by reset.");
            }
            if (!skill8Active) {
                skill8Active = true;
                DEBUG_LOG("--> Capsize monitoring (Skill 8) reactivated by reset.");
            }
            hoverboardInverted = false;
            break;
        case SKILL_TURN_90:
            if (!skill8Active) { DEBUG_LOG("Skill 2 cannot be activated: Capsize protection (Skill 8) is not active."); break; }
            resetAll(); resetHorizon();
            input_EspNowSpeed = 0;
            skillActive = true; currentCase = 2;
            if (param1 > 0) { targetAngle = TURN_ANGLE_90; turnDirection = 1; DEBUG_LOG("Skill 2: 90-degree turn RIGHT triggered!"); }
            else if (param1 < 0) { targetAngle = -TURN_ANGLE_90; turnDirection = -1; DEBUG_LOG("Skill 2: 90-degree turn LEFT triggered!"); }
            targetAngle = normalizeAngle(targetAngle);
            monitorDirection = true;
            break;
        case SKILL_HOLD_LINE:
            if (!skill8Active) { DEBUG_LOG("Skill 3 cannot be activated: Capsize protection (Skill 8) is not active."); break; }
            if (holdLine) {
                resetAll();
                DEBUG_LOG("Skill 3: Deactivated (no longer holding the line).");
            } else {
                resetAll(); holdLine = true; skillActive = true; currentCase = 3;
                resetHorizon(); startYaw = yaw; case3StartTime = millis();
                DEBUG_LOG("Skill 3: Activated (holding the line).");
            }
            break;
        case SKILL_TURN_180:
            if (!skill8Active) { DEBUG_LOG("Skill 4 cannot be activated: Capsize protection (Skill 8) is not active."); break; }
            resetAll(); resetHorizon();
            input_EspNowSpeed = 0;
            skillActive = true; currentCase = 4;
            if (param1 > 0) { targetAngle = TURN_ANGLE_180; turnDirection = 1; DEBUG_LOG("Skill 4: 180-degree turn RIGHT triggered!"); }
            else if (param1 < 0) { targetAngle = -TURN_ANGLE_180; turnDirection = -1; DEBUG_LOG("Skill 4: 180-degree turn LEFT triggered!"); }
            targetAngle = normalizeAngle(targetAngle);
            monitorDirection = true;
            break;
        case SKILL_DISTANCE_MISSION:
            resetMissionFlags = true;
            input_JoySteer = 0;
            input_JoySpeed = 0;
            input_EspNowSteer = (param1 > 0) ? 100 : (param1 < 0 ? -100 : 0);
            input_EspNowSpeed = (param2 > 0) ? 100 : (param2 < 0 ? -100 : 0);
            global_cmdCode = 0;
            Send(input_EspNowSteer, input_EspNowSpeed);
            vTaskDelay(pdMS_TO_TICKS(50));
            DEBUG_PRINTF(">>> MISSION KICKSTARTED & SET: L:%d R:%d\n", param1, param2);
            input_EspNowSteer = param1;
            input_EspNowSpeed = param2;
            global_maxSpeedL = (param3 > 0) ? param3 : currentSettings.currentMaxSpeed;
            global_maxSpeedR = (param4 > 0) ? param4 : currentSettings.currentMaxSpeed;
            global_accel     = (uint16_t)constrain(param5, 0, 100);
            global_brake     = (uint16_t)constrain(param6, 0, 100);
            global_cmdCode    = 2;
            skill20StartTime  = millis();
            skillActive       = true;
            currentSkill      = 20;
            DEBUG_PRINTF(">>> MISSION: L:%dcm R:%dcm | Spd L:%d R:%d | Acc:%d%% Brk:%d%%\n", param1, param2, global_maxSpeedL, global_maxSpeedR, global_accel, global_brake);
            break;
    }
}

// ---------------------------------------------------------------------
// Sub-Handler 2: Blade & Actuator Skills (5, 6, 10, 11)
// ---------------------------------------------------------------------
void handleBladeSkills(uint8_t skill, int16_t param1, int16_t param2, int16_t param3, int16_t param4) {
    switch (skill) {
        case 5:
            if (param1 > 0) {
                // Blade UP
                digitalWrite(BLADE_DOWN_PIN, LOW);
                digitalWrite(BLADE_UP_PIN, HIGH);
                currentBladeHeight = BLADE_HEIGHT_UP;
                currentBladeState = BLADE_OFF;
                digitalWrite(BLADE_UNIT_PIN, LOW);
                bladeEsc.writeMicroseconds(BLADE_ZERO_US);
                DEBUG_LOG("Skill 5: Blade UP (Pin 35 HIGH) - forcing Blade OFF for safety.");
            } else if (param1 < 0) {
                // Blade DOWN
                digitalWrite(BLADE_UP_PIN, LOW);
                digitalWrite(BLADE_DOWN_PIN, HIGH);
                currentBladeHeight = BLADE_HEIGHT_DOWN;
                DEBUG_LOG("Skill 5: Blade DOWN (Pin 36 HIGH).");
            } else {
                // Stop blade movement
                digitalWrite(BLADE_UP_PIN, LOW);
                digitalWrite(BLADE_DOWN_PIN, LOW);
                DEBUG_LOG("Skill 5: Blade Height STOP (Pins LOW).");
            }
            break;
        case SKILL_BLADE_CONTROL: {
            // Blade control: param1/key determines action
            int16_t key = param1;
            
            if (key == BLADE_OFF_CMD) {
                // Blade OFF
                stopBladeSafely("Skill 6: Blade Unit OFF (Pin 19 LOW)");
            } 
            else if (key == BLADE_ON_CMD) {
                // Blade ON
                if (bladeAllowed()) {
                    currentBladeState = BLADE_WORK;
                    digitalWrite(BLADE_UNIT_PIN, HIGH);
                    current_blade_pwm = BLADE_ZERO_US;
                    DEBUG_LOG("Skill 6: Blade Unit ON (Pin 19 HIGH)");
                } else {
                    stopBladeSafely("Skill 6: Blade ON blocked - blade is UP (safety)");
                }
            } 
            else if (key >= PWM_WEB_MIN && key <= PWM_WEB_MAX) {
                // Web PWM control (21000-22000 maps to 1000-2000 PWM)
                int16_t pwmValue = key - PWM_WEB_OFFSET;
                // Validate PWM range (1000-2000 microseconds)
                if (pwmValue < 1000 || pwmValue > 2000) {
                    DEBUG_PRINTF("ERROR: PWM value %d out of valid range (1000-2000)\n", pwmValue);
                    stopBladeSafely("Skill 6: PWM out of range");
                } else if (bladeAllowed()) {
                    currentBladeState = BLADE_WORK;
                    current_blade_pwm = pwmValue;
                    DEBUG_PRINTF("Skill 6: Web-PWM-> %d PWM\n", current_blade_pwm);
                } else {
                    stopBladeSafely("Skill 6: Web-PWM blocked - blade is UP (safety)");
                }
            } 
            else if (key == KEY_BLADE_SPEED1 || key == KEY_BLADE_SPEED1_ALT) {
                // Blade Speed 1 (Working speed)
                if (bladeAllowed()) {
                    currentBladeState = BLADE_WORK;
                    current_blade_pwm = currentSettings.bladeWorkingSpeed;
                    DEBUG_PRINTF("Skill 6: Blade Speed 1 -> %d PWM\n", current_blade_pwm);
                } else {
                    stopBladeSafely("Skill 6: Blade Speed 1 blocked - blade is UP (safety)");
                }
            } 
            else if (key == KEY_BLADE_SPEED2 || key == KEY_BLADE_SPEED2_ALT) {
                // Blade Speed 2 (Stage 2 speed)
                if (bladeAllowed()) {
                    currentBladeState = BLADE_WORK;
                    current_blade_pwm = currentSettings.bladeStage2Speed;
                    DEBUG_PRINTF("Skill 6: Blade Speed 2 -> %d PWM\n", current_blade_pwm);
                } else {
                    stopBladeSafely("Skill 6: Blade Speed 2 blocked - blade is UP (safety)");
                }
            }
            break;
        }
        case SKILL_SET_BLADE_SPEEDS:
            currentSettings.bladeWorkingSpeed = param1;
            currentSettings.bladeStage2Speed = param2;
            currentSettings.bladeMaxSpeed = param3;
            currentSettings.bladeCableResetRpm = param4;
            if (!saveSettings()) {
                DEBUG_LOG("ERROR: Skill 10 - Failed to save blade speeds!");
            }
            DEBUG_PRINTF("Skill 10: Blade speeds updated - Zero: %d, Working pwm: %d, 2 Stage pwm: %d, Max pwm: %d, Reset pwm: %d\n",
                         BLADE_ZERO_US, currentSettings.bladeWorkingSpeed, currentSettings.bladeStage2Speed,
                         currentSettings.bladeMaxSpeed, currentSettings.bladeCableResetRpm);
            break;
        case SKILL_CABLE_RESET:
            DEBUG_LOG("Skill 11: Reset triggered. Stopping blade and starting sequence...");
            current_blade_pwm = BLADE_ZERO_US;
            bladeEsc.writeMicroseconds(BLADE_ZERO_US);
            currentBladeState = BLADE_CABLE_RESET;
            bladeSequenceStartTime = millis();
            if (param1 > MIN_RPM_THRESHOLD) currentSettings.bladeCableResetRpm = param1;
            DEBUG_PRINTF("Skill 11: Blade Reset Sequence started. Target: %d PWM\n",
                         currentSettings.bladeCableResetRpm);
            break;
    }
}

// ---------------------------------------------------------------------
// Sub-Handler 3: Setup & System Skills (7, 8, 9, 12-19)
// ---------------------------------------------------------------------
void handleSetupSkills(uint8_t skill, int16_t param1, int16_t param2, int16_t param3, int16_t param4, int16_t param5, int16_t param6) {
    switch (skill) {
        case SKILL_SET_LIMITS:
            if (param3 > 0) currentSettings.currentMaxSpeed = constrain(param3, 0, MAX_SPEED);
            if (param2 > 0) currentSettings.currentMaxSteer = constrain(param2, 0, MAX_STEER);
            if (!saveSettings()) {
                DEBUG_LOG("ERROR: Skill 7 - Failed to save speed/steer limits!");
            }
            DEBUG_PRINTF("Skill 7: Updated currentMaxSpeed to %d and currentMaxSteer to %d\n", currentSettings.currentMaxSpeed, currentSettings.currentMaxSteer);
            break;
        case SKILL_ESTOP:
            // Config mode: Set capsize angle (param1=CONFIG_MODE_ANGLE)
            if (param1 == CONFIG_MODE_ANGLE) {
                currentSettings.capsizeAngle = (float)param2;
                if (!saveSettings()) {
                    DEBUG_LOG("ERROR: Skill 8 - Failed to save capsize angle!");
                }
                DEBUG_PRINTF("Skill 8: Capsize Angle set to %.2f degrees.\n", currentSettings.capsizeAngle);
                return;
            }
            // Config mode: Set capsize timeout (param1=CONFIG_MODE_TIMEOUT)
            if (param1 == CONFIG_MODE_TIMEOUT) {
                currentSettings.capsizeTimeout = param2;
                if (!saveSettings()) {
                    DEBUG_LOG("ERROR: Skill 8 - Failed to save capsize timeout!");
                }
                DEBUG_PRINTF("Skill 8: Capsize Timeout set to %d ms.\n", currentSettings.capsizeTimeout);
                return;
            }

            if (param1 == 20) {
                // Emergency stop trigger
                resetAll();
                digitalWrite(BLADE_UNIT_PIN, LOW);
                digitalWrite(DRIVE_UNIT_PIN, LOW);
                currentBladeState = BLADE_OFF;
                bladeEsc.writeMicroseconds(BLADE_ZERO_US);
                skill8SafetyActive = true;
                static unsigned long lastEstopPrint = 0;
                unsigned long now = millis();
                if (now - lastEstopPrint > 2000) {
                    DEBUG_LOG("--> IMMEDIATE EMERGENCY STOP TRIGGERED <--");
                    lastEstopPrint = now;
                }
            } else if (param1 > 0) {
                // Activate capsize monitoring
                if (!skill8Active) {
                    resetAll();
                    skill8Active = true;
                    DEBUG_LOG("Skill 8: Activated (Capsize MPU monitoring started).");
                }
            } else if (param1 == -20) {
                // Reset/stop command
                DEBUG_LOG("Stop/Reset command received: Stopping skills, resetting horizon, clearing E-Stop.");
                resetAll();
                resetHorizon();
                if (skill8SafetyActive) {
                    skill8SafetyActive = false;
                    DEBUG_LOG("--> Emergency Stop state has been cleared.");
                }
            } else if (param1 < 0) {
                // Deactivate capsize monitoring
                if (skill8Active) {
                    skill8Active = false;
                    hoverboardInverted = false;
                    skill8SafetyActive = false;
                    DEBUG_LOG("Skill 8: Deactivated (Capsize MPU monitoring stopped).");
                }
            } else {
                DEBUG_LOG("Skill 8: Undefined command, requires a non-zero value.");
            }
            break;
        case SKILL_REBOOT:
            DEBUG_LOG("Skill 9 triggered via Webpage. Restarting ESP32...");
            ESP.restart();
            break;
        case SKILL_DRIVE_LEVELS:
            currentSettings.driveMinShutdownVoltage = param1;
            currentSettings.driveSafetyModeVoltage = param2;
            currentSettings.driveHighVoltage = param3;
            currentSettings.driveEmergencyLowTemp = param4;
            currentSettings.driveSafetyModeTemp = param5;
            currentSettings.driveHighTemp = param6;
            if (!saveSettings()) {
                DEBUG_LOG("ERROR: Skill 12 - Failed to save drive levels!");
            }
            DEBUG_PRINTF("Skill 12: Drive settings updated - V: %d, %d, %d | Temp: %d, %d, %d\n",
                         currentSettings.driveMinShutdownVoltage, currentSettings.driveSafetyModeVoltage,
                         currentSettings.driveHighVoltage, currentSettings.driveEmergencyLowTemp,
                         currentSettings.driveSafetyModeTemp, currentSettings.driveHighTemp);
            break;
        case SKILL_BLADE_LEVELS:
            currentSettings.bladeMinShutdownVoltage = param1;
            currentSettings.bladeSafetyModeVoltage = param2;
            currentSettings.bladeHighVoltage = param3;
            currentSettings.bladeEmergencyLowTemp = param4;
            currentSettings.bladeSafetyModeTemp = param5;
            currentSettings.bladeHighTemp = param6;
            if (!saveSettings()) {
                DEBUG_LOG("ERROR: Skill 13 - Failed to save blade levels!");
            }
            DEBUG_PRINTF("Skill 13: Blade settings updated - V: %d, %d, %d | Temp: %d, %d, %d\n",
                         currentSettings.bladeMinShutdownVoltage, currentSettings.bladeSafetyModeVoltage,
                         currentSettings.bladeHighVoltage, currentSettings.bladeEmergencyLowTemp,
                         currentSettings.bladeSafetyModeTemp, currentSettings.bladeHighTemp);
            break;
        case SKILL_CAPSIZE_PARAMS:
            currentSettings.capsizeAngle = (float)param1;
            currentSettings.capsizeTimeout = param2;
            if (!saveSettings()) {
                DEBUG_LOG("ERROR: Skill 14 - Failed to save capsize parameters!");
            }
            DEBUG_PRINTF("Skill 14: Capsize parameters updated - Angle: %.2f, Timeout: %d\n",
                         currentSettings.capsizeAngle, currentSettings.capsizeTimeout);
            break;
        case SKILL_PD_GAINS:
            // Validate PD gains (reasonable range: 0.01 to 10.0)
            if (param1 > 0) {
                float kp = (float)param1 / 100.0f;
                if (kp > 10.0f) {
                    DEBUG_PRINTF("WARNING: Kp %.2f exceeds max 10.0, clamping.\n", kp);
                    kp = 10.0f;
                }
                currentSettings.Kp = kp;
            }
            if (param2 > 0) {
                float kd = (float)param2 / 100.0f;
                if (kd > 10.0f) {
                    DEBUG_PRINTF("WARNING: Kd %.2f exceeds max 10.0, clamping.\n", kd);
                    kd = 10.0f;
                }
                currentSettings.Kd = kd;
            }
            if (!saveSettings()) {
                DEBUG_LOG("ERROR: Skill 15 - Failed to save PD gains!");
            }
            DEBUG_PRINTF("Skill 15: PD-Regler aktualisiert - Kp: %.2f, Kd: %.2f\n", currentSettings.Kp, currentSettings.Kd);
            break;
        case SKILL_BLADE_BATTERY_FACTOR:
            if (param1 > 0) {
                float factor = (float)param1 / 100.0f;
                // Validate factor (reasonable range: 1.0 to 50.0)
                if (factor < 1.0f || factor > 50.0f) {
                    DEBUG_PRINTF("WARNING: Blade battery factor %.2f outside valid range (1.0-50.0)\n", factor);
                }
                currentSettings.bladeBatteryFactor = factor;
                if (!saveSettings()) {
                    DEBUG_LOG("ERROR: Skill 16 - Failed to save blade battery factor!");
                }
                DEBUG_PRINTF("Neuer Blade Batterie Faktor: %.2f\n", currentSettings.bladeBatteryFactor);
            }
            break;
        case SKILL_DRIVE_BATTERY_FACTOR:
            if (param1 > 0) {
                float factor = (float)param1 / 1000.0f;
                // Validate factor (reasonable range: 0.5 to 5.0)
                if (factor < 0.5f || factor > 5.0f) {
                    DEBUG_PRINTF("WARNING: Drive battery factor %.3f outside valid range (0.5-5.0)\n", factor);
                }
                currentSettings.driveBatteryFactor = factor;
                if (!saveSettings()) {
                    DEBUG_LOG("ERROR: Skill 17 - Failed to save drive battery factor!");
                }
                DEBUG_PRINTF("Neuer Drive Faktor: %.3f\n", currentSettings.driveBatteryFactor);
            }
            break;
        case SKILL_DRIVE_POWER:
            if (param1 > 0) {
                digitalWrite(DRIVE_UNIT_PIN, HIGH);
                DEBUG_LOG("Skill 18: Drive Unit Power ON (Pin 20 HIGH)");
            } else {
                digitalWrite(DRIVE_UNIT_PIN, LOW);
                DEBUG_LOG("Skill 18: Drive Unit Power OFF (Pin 20 LOW)");
            }
            break;
        case SKILL_HOVERBOARD_PARAMS:
            global_cmdCode = (uint16_t)param1;
            global_accel = (uint16_t)param2;
            global_brake = (uint16_t)param3;
            DEBUG_PRINTF("Skill 19: Parameters updated -> Cmd: %d, Accel: %d, Brake: %d\n",
                         global_cmdCode, global_accel, global_brake);
            break;
    }
}

// ---------------------------------------------------------------------
// HAUPT-VERTEILER (Der Ersatz für das Spaghetti-Monster)
// ---------------------------------------------------------------------
void triggerSkill(uint8_t skill, int16_t param1 = 0, int16_t param2 = 0, int16_t param3 = 0, int16_t param4 = 0, int16_t param5 = 0, int16_t param6 = 0) {
    // Validate skill code (0 and >20 are reserved/invalid, except 255)
    if (skill == SKILL_NONE) {
        DEBUG_LOG("WARNING: Skill 0 (none) triggered - ignoring.");
        return;
    }
    if (skill > SKILL_DISTANCE_MISSION && skill != SKILL_HEARTBEAT) {
        DEBUG_PRINTF("WARNING: Invalid skill code %d - ignoring.\n", skill);
        return;
    }
    
    // Verteile Skills an die entsprechenden Handler
    if (skill <= SKILL_TURN_180 || skill == SKILL_DISTANCE_MISSION) {
        // Drive & Movement Skills: 0-4, 20
        handleDriveSkills(skill, param1, param2, param3, param4, param5, param6);
    } else if (skill == SKILL_BLADE_LIFT || skill == SKILL_BLADE_CONTROL || 
               skill == SKILL_SET_BLADE_SPEEDS || skill == SKILL_CABLE_RESET) {
        // Blade & Actuator Skills: 5, 6, 10, 11
        handleBladeSkills(skill, param1, param2, param3, param4);
    } else {
        // Setup & System Skills: 7-9, 12-19, 255
        handleSetupSkills(skill, param1, param2, param3, param4, param5, param6);
    }
}

// --- Die Send-Funktion passt sich dem cmdCode an ---
void Send(int16_t uSteer, int16_t uSpeed) {
    static unsigned long lastHeartbeatSend = 0;
    if (millis() - lastHeartbeatSend < 50) return; 
    lastHeartbeatSend = millis();

    if (isHighVoltageShutdown || skill8SafetyActive) {
        uSteer = 0; uSpeed = 0;
    }

    // *** TASK 1.1: Mutex-Schutz für UART-Kommunikation ***
    if (xSemaphoreTake(bufferMutex, (TickType_t)10) == pdTRUE) {
        Command.start     = 0xABCD;
        Command.cmdCode   = (int16_t)global_cmdCode; 
        
        // *** REFACTORED: Send() verwendet stumpf die übergebenen Parameter ***
        // Die Entscheidung was übergeben wird, liegt beim Aufrufer (controlLogicTask)
        Command.steer = uSteer;
        Command.speed = uSpeed;

        // Zuweisung aller Parameter für die binäre Übertragung
        Command.maxSpeedL = global_maxSpeedL;
        Command.maxSpeedR = global_maxSpeedR; // Sicherstellen, dass R auch gesendet wird
        Command.accPct    = (int16_t)global_accel; 
        Command.brkPct    = (int16_t)global_brake;

        // Checksumme muss exakt alle Felder der SerialCommand Struktur im STM32 spiegeln
        Command.checksum = Command.start ^ 
                           Command.cmdCode ^ 
                           Command.steer ^ 
                           Command.speed ^ 
                           Command.maxSpeedL ^ 
                           Command.maxSpeedR ^ 
                           Command.accPct ^ 
                           Command.brkPct;

        // Binäres Paket an den STM32 feuern
        HoverSerial.write((uint8_t *)&Command, sizeof(SerialCommand));
        
        xSemaphoreGive(bufferMutex);
    }
}



void sendESPNow(uint8_t *mac, ESPNowCommand *command) {
    esp_err_t result;

    // Set start frame
    command->start = START_FRAME;

    // Calculate checksum (excluding the checksum field itself)
    command->checksum = command->start ^ command->steer ^ command->speed ^ command->commandCode;

    // Acquire mutex only for the send operation
    if (xSemaphoreTake(espNowMutex, (TickType_t)20)) { // <-- VON portMAX_DELAY AUF 20 TICKS GEÄNDERT
        result = esp_now_send(mac, (uint8_t *)command, sizeof(ESPNowCommand));
        xSemaphoreGive(espNowMutex);
    } else {
        // Dies ist jetzt kein fataler Fehler mehr, sondern ein erwarteter Timeout, 
        // wenn der Receive-Callback den Mutex gerade hält. Wir verwerfen einfach diesen einen Heartbeat.
        // debugPrintln("WARN: Mutex timeout in sendESPNow, dropping packet."); // Optionales Debugging
        return;
    }

    if (result != ESP_OK) {
      debugPrintf("ESP-NOW send failed with error: %d\n", result);
    }
}


//////////////////////////////ESP_NOW HANDLING//////////////////////////////////////////////////
// KORRIGIERTE FUNKTION: Fügt Mutex-Schutz für peers[] Array hinzu
// ====================================================================================
// ===== FIX: Robuste onDataReceive Funktion für Receiver (Gehärtet) ==================
// ====================================================================================
void onDataReceive(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
    
    // 1. Sichere lokale Kopie anlegen (WICHTIG für Speicher-Alignment)
    ESPNowCommand cmd;

    // 2. Längen-Check
    if (len != sizeof(ESPNowCommand)) {
        // debugPrintf("ESP-NOW Error: Wrong Length. Recv=%d, Expected=%d\n", len, sizeof(ESPNowCommand));
        return; 
    }

    // 3. Daten kopieren (Sicherer als Pointer-Casting)
    memcpy(&cmd, incomingData, sizeof(cmd));

    // 4. Start-Frame Check
    if (cmd.start != START_FRAME) {
        // debugPrintf("ESP-NOW Error: Invalid Start Frame. Recv=0x%04X\n", cmd.start);
        return;
    }

    // 5. Checksummen-Berechnung (STRIKTES Casting auf uint16_t)
    uint16_t calculatedChecksum = cmd.start ^ (uint16_t)cmd.steer ^ (uint16_t)cmd.speed ^ (uint16_t)cmd.commandCode;

    if (calculatedChecksum != cmd.checksum) {
        debugPrintf("INVALID FRAME: calc=0x%04X recv=0x%04X\n", calculatedChecksum, cmd.checksum);
        return; // <--- GANZ WICHTIG: Hier sofort abbrechen!
    }

    // --- Peer Handling (Mutex geschützt) ---
    int senderIndex = -1; 
    if (xSemaphoreTake(espNowMutex, (TickType_t)20) == pdTRUE) {
        for (int i = 0; i < numberOfPeers; i++) {
            if (memcmp(mac_addr, deviceList[i].mac, 6) == 0) {
                senderIndex = i; 
                if (!peers[senderIndex].isConnected) {
                    peers[senderIndex].isConnected = true;
                    peers[senderIndex].reconnectAttempts = 0;
                    DEBUG_PRINTF("Peer %d connected via ESP-NOW.\n", deviceList[senderIndex].id);
                }
                peers[senderIndex].lastSeen = millis();
                break;           
            }
        } 
        xSemaphoreGive(espNowMutex);
    } else {
        return; // Mutex Timeout
    }

    if (senderIndex == -1) return; // Unbekannter Sender

    // --- Befehlsverarbeitung ---
    // HINWEIS: input_EspNow* und input_Joy* Variablen sind 'volatile int16_t'.
    // Auf ESP32 sind aligned 16-bit Lese/Schreib-Operationen hardware-atomar.
    // Mutex-Schutz hier weggelassen für Performance (worst case: 1 veralteter Wert pro 30ms Zyklus).
    switch (cmd.commandCode) {
        case 0:  // Fahrbefehl
            input_EspNowSteer = cmd.steer;
            input_EspNowSpeed = cmd.speed;
            input_JoySteer = cmd.steer;   // Separater Speicher für Override-Check
            input_JoySpeed = cmd.speed;
            lastJoystickCommandTime = millis();
            break;
            
        case 255: // Heartbeat
            break;

        default: // Alle Skills (1-17, etc.)
            DEBUG_PRINTF("Skill CMD %d Recv -> Steer(Param1): %d, Speed(Param2): %d\n", cmd.commandCode, cmd.steer, cmd.speed);
            triggerSkill(cmd.commandCode, cmd.steer, cmd.speed);
            break;
    }
}

TaskHandle_t heartbeatTaskHandle = NULL;

void heartbeatTask(void *parameter) {
    // Erstelle ein wiederverwendbares Heartbeat-Befehlspaket.
    ESPNowCommand heartbeatCmd;
    heartbeatCmd.commandCode = 255; // 255 ist der designierte Heartbeat-Code.
    heartbeatCmd.steer = 0;
    heartbeatCmd.speed = 0;
    // Die sendESPNow-Funktion fügt automatisch den START_FRAME und die Checksum hinzu.

    while (true) {
        // 1. Prüfe auf Timeouts von anderen Peers (dies erledigt managePeers).
        managePeers();

        // 2. Sende unseren eigenen Heartbeat an alle registrierten Peers,
        //    um zu zeigen, dass wir noch aktiv sind.
        //    Dies löst das Problem, dass die Remote dieses Gerät als offline markiert.
        for (int i = 0; i < numberOfPeers; i++) {
        if (deviceList[i].id != deviceID) { 
            // Nur senden, wenn wir von diesem Peer länger als 2 Sek. nichts gehört haben
            if (millis() - peers[i].lastSeen > 2000) {
                sendESPNow(deviceList[i].mac, &heartbeatCmd);
                vTaskDelay(pdMS_TO_TICKS(15)); // Etwas mehr Zeit für den Funkkanal
            }
        }
        }

        // 3. Warte auf das nächste Intervall.
        vTaskDelay(HEARTBEAT_INTERVAL / portTICK_PERIOD_MS);
    }
}
    

// Helper function to check if a string is a valid integer
bool isValidInteger(String str) {
    str.trim();
    if (str.length() == 0) return false;
    for (unsigned int i = 0; i < str.length(); i++) {
        if (i == 0 && (str.charAt(i) == '-' || str.charAt(i) == '+')) {
            if (str.length() == 1) return false; // Only sign, no digits
            else continue;
        }
        if (!isDigit(str.charAt(i))) return false;
    }
    return true;
}

void handleSerialInput() {
    static String inputString = "";     // A static variable to hold the incoming data
    static bool stringComplete = false; // A flag to indicate when a full line has been received

    // 1. Daten einlesen
    while (Serial.available()) {
        char inChar = (char)Serial.read();
        if (inChar == '\n') {
            stringComplete = true;
        } else if (inChar != '\r') {
            inputString += inChar;
        }
    }

    // 2. Befehl verarbeiten, wenn vollständig
    if (stringComplete) {
        inputString.trim(); // Leerzeichen entfernen

        // --- A: RESET BEFEHL ---
        if (inputString.equalsIgnoreCase("reset")) {
            DEBUG_LOG("Serial Reset Command Received. Restarting ESP32...");
            ESP.restart();
        }
        
        // --- B: KOMMA-BEFEHLE (Parameter) ---
        else if (inputString.indexOf(',') != -1) {
            int commaIndex1 = inputString.indexOf(',');
            String commandStr = inputString.substring(0, commaIndex1);
            String remainingStr = inputString.substring(commaIndex1 + 1);

            commandStr.trim();
            remainingStr.trim();

            // Skill 7: Limits setzen (Format: "7, speed, steer")
            if (commandStr == "7") { 
                int commaIndex2 = remainingStr.indexOf(',');
                if (commaIndex2 != -1) {
                    String speedStr = remainingStr.substring(0, commaIndex2);
                    String steerStr = remainingStr.substring(commaIndex2 + 1);
                    speedStr.trim();
                    steerStr.trim();
                    
                    if (isValidInteger(speedStr) && isValidInteger(steerStr)) {
                        triggerSkill(7, 0, steerStr.toInt(), speedStr.toInt());
                    } else {
                        debugPrintln("Error: Skill 7 parameters must be integers.");
                    }
                }
            } 
            // Skill 16: Blade Faktor setzen (Format: "16, 1940")
            else if (commandStr == "16") {
                if (isValidInteger(remainingStr)) {
                    int16_t factorValue = remainingStr.toInt();
                    triggerSkill(16, factorValue); 
                } else {
                    debugPrintln("Error: Skill 16 value must be an integer.");
                }
            }
            // Skill 20: Präzisionsfahrt (Format: "20, L_cm, R_cm")
            else if (commandStr == "20") {
                int commaIndex2 = remainingStr.indexOf(',');
                if (commaIndex2 != -1) {
                    String leftStr = remainingStr.substring(0, commaIndex2);
                    String rightStr = remainingStr.substring(commaIndex2 + 1);
                    leftStr.trim();
                    rightStr.trim();
                    if (isValidInteger(leftStr) && isValidInteger(rightStr)) {
                        triggerSkill(20, leftStr.toInt(), rightStr.toInt());
                    } else {
                        debugPrintln("Error: Skill 20 parameters must be integers.");
                    }
                }
            }
            // Skill 19: Neue Parameter setzen (Format: "19, cmd, accel, brake")
            else if (commandStr == "19") {
                // Wir erwarten 2 weitere Kommas im restlichen String
                int commaIndex2 = remainingStr.indexOf(',');
                if (commaIndex2 != -1) {
                    String sCmd = remainingStr.substring(0, commaIndex2);
                    String rest = remainingStr.substring(commaIndex2 + 1);
                    
                    int commaIndex3 = rest.indexOf(',');
                    if (commaIndex3 != -1) {
                        String sAccel = rest.substring(0, commaIndex3);
                        String sBrake = rest.substring(commaIndex3 + 1);
                        
                        sCmd.trim(); sAccel.trim(); sBrake.trim();
                        
                        if (isValidInteger(sCmd) && isValidInteger(sAccel) && isValidInteger(sBrake)) {
                            triggerSkill(19, sCmd.toInt(), sAccel.toInt(), sBrake.toInt());
                        } else {
                            debugPrintln("Error: Skill 19 params must be integers (19, cmd, acc, brk).");
                        }
                    }
                }
            }
            // Normaler Fahrbefehl (Format: "steer, speed")
            else { 
                if (isValidInteger(commandStr) && isValidInteger(remainingStr)) {
                    int16_t steer = commandStr.toInt();
                    int16_t speed = remainingStr.toInt();
                    
                    // Sicherheits-Check gegen Überlauf beim manuellen Fahren
                    if (steer < -currentSettings.maxSteer || steer > currentSettings.maxSteer) {
                        debugPrintf("Error: Steer value out of range (-%d to %d).\n", currentSettings.maxSteer, currentSettings.maxSteer);
                    } else if (speed < -currentSettings.currentMaxSpeed || speed > currentSettings.currentMaxSpeed) {
                        debugPrintf("Error: Speed value out of range (-%d to %d).\n", currentSettings.currentMaxSpeed, currentSettings.currentMaxSpeed);
                    } else {
                        DEBUG_PRINTF("Sending Command - Steer: %d, Speed: %d\n", steer, speed);
                        Send(steer, speed);
                    }
                } else {
                    debugPrintln("Error: Steer and speed must be valid integers.");
                }
            }
        }

        // --- C: EINZEL-BEFEHLE (Skills ohne Komma) ---
        else if (inputString == "1")  { triggerSkill(SKILL_RESET); }
        else if (inputString == "2-") { triggerSkill(SKILL_TURN_90, -1000); }
        else if (inputString == "2+") { triggerSkill(SKILL_TURN_90, 1000); }
        else if (inputString == "3")  { triggerSkill(SKILL_HOLD_LINE); }
        else if (inputString == "4-") { triggerSkill(SKILL_TURN_180, -1000); }
        else if (inputString == "4+") { triggerSkill(SKILL_TURN_180, 1000); }
        else if (inputString == "6")  { triggerSkill(SKILL_BLADE_CONTROL, BLADE_ON_CMD); }
        else if (inputString == "7")  {
            DEBUG_PRINTF("Skill 7 Info: Current Max Speed: %d, Max Steer: %d\n", currentSettings.currentMaxSpeed, currentSettings.currentMaxSteer);
        }
        else if (inputString == "8")  { triggerSkill(SKILL_ESTOP, 1); }
        else if (inputString == "9")  {
            DEBUG_LOG("Skill 9 triggered via Serial. Restarting ESP32...");
            ESP.restart();
        }

        // --- D: FEHLERHAFTE EINGABE ---
        else {
            if (inputString.length() > 0) {
                debugPrintln("Error: Invalid command format.");
                debugPrintln("Use 'reset', '1'-'9', '16, value', or 'steer,speed'.");
            }
        }

        // String zurücksetzen für nächsten Durchlauf
        inputString = "";
        stringComplete = false;
    }
}



// ====================================================================================
// ===== CHANGE START: Updated function to use configurable settings ================
// ====================================================================================
void monitorSkill8Angles(float pitch, float roll) {
    // Check if the angle has exceeded the configured threshold
    if (!skill8SafetyActive && (abs(pitch) > currentSettings.capsizeAngle || abs(roll) > currentSettings.capsizeAngle)) {
        // This is a potential capsize event
        unsigned long now = millis();

        if (capsizeDetectTime == 0) {
            // This is the FIRST moment we detected the tilt. Start the timer.
            capsizeDetectTime = now;
        } else if (now - capsizeDetectTime > currentSettings.capsizeTimeout) {
            // The tilt has been continuous for longer than our configured timeout. This is a REAL event.
            if (ENABLE_DEBUG_SERIAL) {
                debugPrintf("Skill 8: Capsize safety stop triggered (Angle > %.1f for > %dms).\n", currentSettings.capsizeAngle, currentSettings.capsizeTimeout);
            }
            skill8SafetyActive = true;  // Activate the safety stop
            triggerSkill(SKILL_BLADE_CONTROL, BLADE_OFF_CMD);  // Turn off blade
            Send(0, 0);               // Stop hoverboard movement
        }
        // If the tilt is less than the timeout, we do nothing and wait for the next check.

    } else {
        // Angle is normal. Reset the timer.
        capsizeDetectTime = 0;
    }
}
// ====================================================================================
// ===== CHANGE END: End of updated capsize monitoring function =====================
// ====================================================================================


void IRAM_ATTR dmpDataReady() {
  // We MUST use the "FromISR" version inside an interrupt
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  
  // Give the semaphore to wake up the mpuReadTask
  xSemaphoreGiveFromISR(mpuSemaphore, &xHigherPriorityTaskWoken);

  // If giving the semaphore woke up a task with a higher priority than the one
  // currently running, we must force a context switch.
  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}

void setupWiFiAP() {
    
    WiFi.mode(WIFI_AP_STA);

    const char* ssid = "Lawnmower_Control";
    const char* password = "123456789";
    IPAddress local_IP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);

    // 2. Statische IP-Konfiguration für schnelleren Verbindungsaufbau (DHCP-Fix)
    if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
        DEBUG_LOG("[WiFiAP] AP Config Failed");
    }

    // 3. Start des Access Points auf Kanal 6 (Wichtig für ESP-NOW Kompatibilität)
    // Start the Access Point
    if (WiFi.softAP(ssid, password, 6, 0, 4)) {
        // Starte den DNS Server und leite alle (*) Anfragen auf unsere eigene IP um
        dnsServer.start(DNS_PORT, "*", local_IP);

        DEBUG_LOG("[WiFiAP] Access Point started successfully");
        DEBUG_PRINTF("[WiFiAP] SSID: %s | Password: %s\n", ssid, password);
        DEBUG_PRINTF("[WiFiAP] AP IP Address: %s\n", WiFi.softAPIP().toString().c_str());
    } else {
        DEBUG_LOG("[WiFiAP] Failed to start Access Point!");
    }
}

// =========================================================
// --- VOLLSTÄNDIGE BLADE CONTROL LOGIC (STABILISIERT) ---
// =========================================================
void runBladeLogic() {
    static unsigned long lastPwmUpdate = 0;
    unsigned long currentMillis = millis();

    // 1. TAKTUNG: Logik und PWM-Ausgabe nur alle 20ms
    if (currentMillis - lastPwmUpdate < 20) return; 
    lastPwmUpdate = currentMillis;

    // 2. SICHERHEITS-CHECK
    if (skill8SafetyActive || isInSafetyMode) {
        currentBladeState = BLADE_OFF;
    }

    // 3. STATE MACHINE
    switch (currentBladeState) {
        case BLADE_OFF:
            current_blade_pwm = BLADE_ZERO_US; 
            if (bladeEsc.attached()) bladeEsc.detach(); 
            digitalWrite(BLADE_ESC_PIN, LOW);                 
            break;

        case BLADE_WORK:
            if (!bladeEsc.attached()) {
                bladeEsc.attach(BLADE_ESC_PIN, 1000, 2000);
            }
            break;

        case BLADE_CABLE_RESET:
            // Sobald Energie für das Messer gebraucht wird, Pin 20 hochziehen
            if (digitalRead(BLADE_UNIT_PIN) == LOW) {
                digitalWrite(BLADE_UNIT_PIN, HIGH);
                DEBUG_LOG("Blade Logic: Unit Power ON (Pin 20)");
            }
            
            if (!bladeEsc.attached()) {
                bladeEsc.attach(BLADE_ESC_PIN, 1000, 2000);
            }
           

            unsigned long timeInSeq = currentMillis - bladeSequenceStartTime;
            int targetPwm = currentSettings.bladeCableResetRpm;
            if (targetPwm <= 1500) targetPwm = 1600; 

            if (timeInSeq <= RESET_RAMP_MS) {
                float progress = (float)timeInSeq / (float)RESET_RAMP_MS;
                int range = targetPwm - BLADE_ZERO_US;
                current_blade_pwm = BLADE_ZERO_US + (int)(progress * (float)range);
            }
            else if (timeInSeq <= (RESET_RAMP_MS + RESET_HOLD_MS)) {
                current_blade_pwm = targetPwm;
            }
            else {
                current_blade_pwm = BLADE_ZERO_US;
                currentBladeState = BLADE_WORK;
                DEBUG_LOG("Blade Reset Sequence finished.");
            }
            break;
    }

    // 4. HARDWARE-AUSGABE
    if (bladeEsc.attached() && currentBladeState != BLADE_OFF) {
        int pulse = current_blade_pwm;
        if (pulse > 2000) pulse = 2000;
        if (pulse < 1000) pulse = 1000;
        
        bladeEsc.writeMicroseconds(pulse);
    }
} 

void setup() {
    Serial.begin(SERIAL_BAUD);

    // *** SICHERHEIT: SerialFeedback Struct-Größe prüfen ***
    if (sizeof(SerialFeedback) != 18) {
        Serial.printf("FATAL: SerialFeedback struct size is %d, expected 18!\n", sizeof(SerialFeedback));
        // In Entwicklung: endlos warten um Fehler zu sehen
        // In Produktion: trotzdem starten aber loggen
    }

    // *** NOT-AUS Pin Initialisierung ***
    pinMode(EMERGENCY_STOP_PIN, INPUT_PULLUP); // Emergency stop button, active LOW

    pinMode(BLADE_UP_PIN, OUTPUT);   // Nutzt jetzt Pin 35
    pinMode(BLADE_DOWN_PIN, OUTPUT); // Nutzt jetzt Pin 36
    digitalWrite(BLADE_UP_PIN, LOW);
    digitalWrite(BLADE_DOWN_PIN, LOW);

    // --- Blade ESC Initialisierung ---
    // Standard ESC: 1000us = 0%, 2000us = 100%
    bladeEsc.attach(BLADE_ESC_PIN, 1000, 2000);
    bladeEsc.writeMicroseconds(BLADE_ZERO_US); // Motor sicher auf AUS stellen

    pinMode(BLADE_UNIT_PIN, OUTPUT);
    digitalWrite(BLADE_UNIT_PIN, LOW);   // Pin 19 auf LOW beim Start

    pinMode(DRIVE_UNIT_PIN, OUTPUT);
    digitalWrite(DRIVE_UNIT_PIN, LOW);   // Pin 20 auf LOW beim Start

    if (ENABLE_DEBUG_SERIAL) {
        debugPrintln("Blade ESC initialized on Pin 4.");
        debugPrintln("Blade Unit (19) and Drive Unit (20) initialized as LOW.");
    }

    // Initialize EEPROM and load settings
    EEPROM.begin(EEPROM_SIZE);
    loadSettings();

    // Initialize peer tracking structure
    unsigned long currentMillis = millis();
    for (int i = 0; i < numberOfPeers; i++) {
        peers[i].isConnected = false;
        peers[i].lastSeen = currentMillis;
        peers[i].reconnectAttempts = 0;
        peers[i].lastReconnectAttempt = 0;
    }

    // Read Wi-Fi credentials from EEPROM
    readWiFiCredentialsFromEEPROM();

    // 1. WiFi-Hardware resetten (verhindert Channel-Konflikte mit ESP-NOW)
    WiFi.disconnect(true); 
    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    // 1. AP starten (Erzwingt Kanal 6)
    setupWiFiAP();

    // 2. Heimnetzwerk verbinden (Wieder aktiviert!)
    // Wir nutzen connectToWiFi, da diese Funktion wartet, bis die Verbindung steht.
    if (strlen(stored_ssid) > 0 && isprint(stored_ssid[0])) { // Prüfung auf > 0 (nicht 1)
        DEBUG_PRINTF("[Setup] Gespeicherte SSID gefunden: %s\n", stored_ssid);
        DEBUG_LOG("[Setup] Starte Verbindung zum Heimnetz...");
        connectToWiFi(stored_ssid, stored_password);
    } else {
        DEBUG_LOG("[Setup] Keine WiFi-Daten im Speicher gefunden.");
    }



    // Print WiFi channel information
    DEBUG_PRINTF("Current WiFi Channel: %d | Mode: %s\n",
        WiFi.channel(),
        WiFi.getMode() == WIFI_MODE_AP ? "AP" :
        WiFi.getMode() == WIFI_MODE_STA ? "STA" :
        WiFi.getMode() == WIFI_MODE_APSTA ? "AP+STA" : "Unknown");

    delay(100);


    pinMode(LED_BUILTIN, OUTPUT);

    xTaskCreatePinnedToCore(
            core1WiFiTask,
            "WiFiTask",
            10000,
            NULL,
            5,    
            NULL,
            1
        );

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        debugPrintln("Error initializing ESP-NOW");
        return;
    }
    DEBUG_LOG("ESP-NOW Initialized");

    espNowMutex = xSemaphoreCreateMutex();
    if (espNowMutex == NULL) {
        debugPrintln("Failed to create ESP-NOW mutex");
    }


    // Add peers
    esp_now_peer_info_t peerInfo = {};
    for (int i = 0; i < numberOfPeers; i++) {
        const uint8_t *macAddress = deviceList[i].mac;

        memset(&peerInfo, 0, sizeof(peerInfo));
        memcpy(peerInfo.peer_addr, macAddress, 6);
        peerInfo.channel = 6; // 0 means use current channel
        peerInfo.encrypt = false;

        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
            debugPrintf("Failed to add peer %d\n", i);
        } else {
            DEBUG_PRINTF("Added peer %d successfully\n", i);
        }
    }


  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BLADE_BATTERY_PIN, INPUT);

  // Serial starten
  HoverSerial.begin(115200, SERIAL_8N1, HOVER_RX_PIN, HOVER_TX_PIN);

  // SOFORT "Ich bin da, bleib stehen" senden, um Timeout-Piepen zu verhindern
  // Wir senden das ein paar Mal, um sicherzugehen, dass der Puffer synchronisiert ist.
  for(int i=0; i<5; i++) {
      Send(0, 0); 
      delay(20);
  }

  i2cMutex = xSemaphoreCreateMutex();
    if (i2cMutex == NULL) {
        debugPrintln("Konnte i2cMutex nicht erstellen!");
    }
    
    feedbackMutex = xSemaphoreCreateMutex();
    if (feedbackMutex == NULL) {
        debugPrintln("Konnte feedbackMutex nicht erstellen!");
    }
    
    mpuSemaphore = xSemaphoreCreateBinary();
    if (mpuSemaphore == NULL) {
        debugPrintln("FATAL: Could not create MPU semaphore!");
    }



      Wire.begin(13, 14); // SDA, SCL for some ESP32 boards
      Wire.setClock(100000); // Setzt I2C auf 100kHz
      Wire.setTimeOut(20); // 20ms Hardware-Timeout für I2C-Hänger

      // ====================================================================================
      // ===== CHANGE START: Robuste MPU-Init-Retry-Schleife (auf 2 Versuche limitiert) ===
      // ====================================================================================

      const int MAX_MPU_ATTEMPTS = 2; // *** WUNSCHGEMÄSS GEÄNDERT: Nur 2 Versuche ***
      int rectangleMpuAttempts = 0;
      bool rectangleMpuInitialized = false; // Flag zur Verfolgung des Erfolgs

    while (rectangleMpuAttempts < MAX_MPU_ATTEMPTS && !rectangleMpuInitialized) {
        rectangleMpuAttempts++;
        DEBUG_PRINTF("Versuche MPU-Initialisierung (Versuch %d/%d)...\n", rectangleMpuAttempts, MAX_MPU_ATTEMPTS);

        mpu.initialize();

        if (!mpu.testConnection()) {
            DEBUG_LOG(" -> MPU6050-Verbindungstest fehlgeschlagen.");
            delay(500); // Warte vor dem nächsten Verbindungsversuch
            continue;   // Springe zur nächsten Schleifeniteration
        }

          // Verbindung OK, versuche nun, die DMP zu initialisieren
          devStatus = mpu.dmpInitialize();
          
        if (devStatus == 0) {
            // ERFOLG!
            mpu.setDMPEnabled(true);
            dmpReady = true;
            packetSize = mpu.dmpGetFIFOPacketSize();
            DEBUG_LOG(" -> MPU6050 DMP erfolgreich initialisiert.");
            pinMode(46, INPUT); // GPIO 46 als Eingang für den INT-Pin
            attachInterrupt(digitalPinToInterrupt(46), dmpDataReady, RISING); // ISR an Pin 46 binden
            mpu.setIntDataReadyEnabled(true); // Dem MPU sagen, dass er den Interrupt-Pin nutzen soll

            rectangleMpuInitialized = true; // Setze Flag, um die Schleife zu verlassen
        } else {
            // DMP-Init fehlgeschlagen
            DEBUG_PRINTF(" -> MPU6050 DMP-Initialisierung fehlgeschlagen mit Code: %d. Wiederhole...\n", devStatus);
            delay(500); // Warte vor dem erneuten Versuch der DMP-Init
        }
    } // Ende der while-Schleife

    // Prüfe nach der Schleife, ob wir endgültig gescheitert sind
    if (!rectangleMpuInitialized) {
        debugPrintln("KRITISCHER FEHLER: MPU6050 konnte nach 2 Versuchen nicht initialisiert werden! MPU-basierte Skills sind deaktiviert.");
    }
    // ====================================================================================
    // ===== CHANGE END: Ende der MPU-Init-Retry-Schleife ================================
    // ====================================================================================

    // ====================================================================================
    // ===== CHANGE START: Initial Horizon Calibration on Startup & Task Creation Order ===
    // ====================================================================================
    if (rectangleMpuInitialized) {
        // Set DMP update rate to 50Hz (1kHz / (1 + 19) = 50Hz)
        // Halbiert die I2C Last, reicht für Kippschutz völlig aus.
        mpu.setRate(19);
        DEBUG_LOG("MPU DMP rate set to 50Hz for I2C relief.");

          // Create and start the MPU task IMMEDIATELY after initialization
          // to ensure the FIFO buffer is being cleared.
          xTaskCreatePinnedToCore(
              mpuReadTask,
              "MPUTask",
              4096,
              NULL,
              0,                    // Priorität erhöht für zuverlässigen Kippschutz
              NULL,
              0                     
          );

          DEBUG_LOG("MPU initialized. Waiting for sensor to stabilize before setting initial horizon...");

          // Give the MPU task time to read stable values.
          vTaskDelay(2000 / portTICK_PERIOD_MS); // Increased delay to 2 seconds
          resetHorizon();
          DEBUG_LOG("Initial horizon set on startup.");
      }
      // ====================================================================================
      // ===== CHANGE END: End of MPU startup logic fixes ===================================
      // ====================================================================================

    // Initialize default skill states

    if (DEFAULT_SKILL1_ACTIVE) {
        triggerSkill(SKILL_RESET); // Skill 1: Reset Gyro Values
    }
    if (DEFAULT_SKILL2_ACTIVE) {
        // Specify direction: 1 for right, -1 for left
        triggerSkill(SKILL_TURN_90, 1); // Skill 2: 90-degree turn right
    }
    if (DEFAULT_SKILL3_ACTIVE) {
        triggerSkill(SKILL_HOLD_LINE); // Skill 3: Hold the Line
    }
    if (DEFAULT_SKILL4_ACTIVE) {
        // Specify direction: 1 for right, -1 for left
        triggerSkill(SKILL_TURN_180, 1); // Skill 4: 180-degree turn right
    }
    if (DEFAULT_SKILL5_ACTIVE) {
        triggerSkill(SKILL_BLADE_LIFT); // Skill 5: Blade Lift
    }
    if (DEFAULT_SKILL6_ACTIVE) {
        // Specify direction: 1 to activate, -1 to deactivate
        triggerSkill(SKILL_BLADE_CONTROL, BLADE_ON_CMD); // Skill 6: Blade On
    }
    if (DEFAULT_SKILL7_ACTIVE) {
        triggerSkill(SKILL_SET_LIMITS); // Skill 7: Max Speed and Steer setting
    }
    if (ENABLE_DEBUG_SERIAL) {
        debugPrintln("Activating default skill: Skill 8 (Capsize Protection).");
    }
    triggerSkill(SKILL_ESTOP, 1); // Force Skill 8 to be active on startup
    if (DEFAULT_SKILL9_ACTIVE) {
        triggerSkill(SKILL_REBOOT); // Skill 9: Reset ESP32
    }



    WiFi.macAddress(selfMacAddress);

      // Print the current device's MAC address for debugging
      if (ENABLE_DEBUG_SERIAL) {
        debugPrintf("ESP32 MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                      selfMacAddress[0], selfMacAddress[1], selfMacAddress[2],
                      selfMacAddress[3], selfMacAddress[4], selfMacAddress[5]);
      }

      // Check against deviceList
      for (int i = 0; i < sizeof(deviceList) / sizeof(deviceList[0]); i++) {
          if (memcmp(selfMacAddress, deviceList[i].mac, 6) == 0) {
              deviceID = deviceList[i].id;
              if (ENABLE_DEBUG_SERIAL) {
                debugPrint("Device ID assigned: ");
                debugPrintln(deviceID);
              }
              break;
          }
      }

      if (deviceID == -1) {
          debugPrintln("Error: Device ID not found in device list. Please update deviceList.");
          // Optionally, handle this case gracefully:
          // Initialize as a default ID or perform additional setup.
      }



      xTaskCreatePinnedToCore(
        heartbeatTask,        // Task function
        "HeartbeatTask",      // Task name
        4096,                  // Stack size
        NULL,                  // Parameters
        0,                     // Priority 
        &heartbeatTaskHandle, // Task handle
        0                      // Core 0
    );

    xTaskCreatePinnedToCore(
        espNowTask,
        "ESPNowTask",
        4096,
        NULL,
        5,                    
        NULL,
        0                      
    );

    
    xTaskCreatePinnedToCore(statusTask, "StatusTask", 4096, NULL, 2, NULL, 0); // Prio 2, Kern 0
    
    // Task für die reine Fahr-Logik (Absolute Priorität auf Kern 1)
    xTaskCreatePinnedToCore(controlLogicTask, "ControlTask", 4096, NULL, 4, NULL, 1);
    
    // Task 5.5: EEPROM Speicher-Task (niedrige Priorität, Core 0)
    xTaskCreatePinnedToCore(eepromSaveTask, "EepromTask", 2048, NULL, 1, NULL, 0);

    // Letzter Befehl: Kanal 6 erzwingen, falls WiFi.begin() ihn verstellt hat
    vTaskDelay(200 / portTICK_PERIOD_MS);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);

    ArduinoOTA.setHostname("Lawnmower-ESP32");
    ArduinoOTA.setPassword("1234"); // Setzt dein gewünschtes Passwort
    
    
    ArduinoOTA.onStart([]() {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        
        // 1. Kompletter System-Stopp (entspricht deinem Skill 1)
        resetAll(); // Stoppt alle Skills und setzt Steer/Speed intern zurück
        resetHorizon(); // Kalibriert Gyro neu
        
        // 2. Sicherheit für Hardware
        Send(0, 0); // Schickt sofort 0/0 an das Hoverboard
        currentBladeState = BLADE_OFF; // Messer hart 
        digitalWrite(BLADE_UNIT_PIN, LOW); // Hardware-Lock
        
        // 3. MPU/I2C Entlastung
        // Wir setzen skill8Active auf false, damit mpuReadTask keine Logik mehr ausführt
        skill8Active = false; 
        
        debugPrintln("OTA Update Start: " + type + " - System in Safety Idle.");
    });

    ArduinoOTA.onEnd([]() {
        debugPrintln("\nUpdate erfolgreich beendet. Starte neu...");
    });

    ArduinoOTA.onError([](ota_error_t error) {
        debugPrintf("OTA Fehler [%u]\n", error);
        // Bei Fehler: Skill 8 wieder an, damit der Mäher nicht ungeschützt bleibt
        skill8Active = true; 
    });

    ArduinoOTA.begin();
    debugPrintln("OTA Service ready (1.9MB Mode)");
}

// =========================================================
// --- VOLLSTÄNDIGE CONTROL LOGIC TASK (60s TIMER & OVERRIDE) ---
// =========================================================
// =========================================================
// --- VOLLSTÄNDIGE CONTROL LOGIC TASK (60s TIMER & OVERRIDE) ---
// =========================================================
void controlLogicTask(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const unsigned long JOYSTICK_TIMEOUT = 5000;
    
    // Status-Variablen für den Handshake
    static bool missionRequestSent = false; 
    static bool missionTriggeredOnce = false;

    for (;;) {
        // *** CRITICAL FIX: Reset flags wenn neue Mission startet ***
        if (resetMissionFlags) {
            missionRequestSent = false;
            missionTriggeredOnce = false;
            resetMissionFlags = false;
            DEBUG_LOG(">> MISSION FLAGS RESET");
        }

        int16_t taskSteer = 0;
        int16_t taskSpeed = 0;
        bool shouldSend = true; 

        // ---------------------------------------------------------
        // 1. MISSIONSLOGIK (Skill 20 / Distanzfahrt)
        // ---------------------------------------------------------
        if (skillActive && currentSkill == 20) {
            
            // *** KRITISCH: E-Stop hat VORRANG vor Mission ***
            if (skill8SafetyActive) {
                skillActive = false;
                currentSkill = 0;
                global_cmdCode = 0;
                missionTriggeredOnce = false;
                missionRequestSent = false;
                resetAll();
                shouldSend = false;
                DEBUG_LOG(">> MISSION ABORT: E-Stop (Skill 8) triggered!");
            }
            // A. Sicherheit: Manueller Abbruch NUR durch echte Joystick-Bewegung
            // input_EspNowSpeed enthält hier 300 (Mission), input_JoySpeed enthält 0.
            if (abs(input_JoySpeed) > 100 || abs(input_JoySteer) > 100) {
                skillActive = false; 
                global_cmdCode = 0;
                missionTriggeredOnce = false;
                missionRequestSent = false;
                shouldSend = true; 
                DEBUG_LOG(">> MISSION ABORT: Real Joystick Override detected.");
            }
            else {
                // B. MISSIONS-START: Befehl genau EINMAL senden
                if (!hoverboardIsBusy && !missionTriggeredOnce && !missionRequestSent) {
                    global_cmdCode = 2;
                    Send(input_EspNowSteer, input_EspNowSpeed); 
                    
                    missionRequestSent = true; 
                    shouldSend = false;        
                    DEBUG_LOG(">> MISSION START: One-Shot Sent. Waiting for ACK...");
                }
                // C. WARTEMODUS: ESP32 schweigt und wartet auf die Busy-Flagge vom STM32
                else if (missionRequestSent && !hoverboardIsBusy && !missionTriggeredOnce) {
                    shouldSend = false; 
                }
                // D. MISSION LÄUFT: STM32 hat die Flagge gesetzt
                else if (hoverboardIsBusy) {
                    shouldSend = false; 
                    missionTriggeredOnce = true; 
                }
                // E. ZIEL ERREICHT: STM32 meldet Ende der Fahrt (Busy wieder false)
                else if (!hoverboardIsBusy && missionTriggeredOnce) {
                    skillActive = false;
                    currentSkill = 0;
                    global_cmdCode = 0;
                    missionTriggeredOnce = false;
                    missionRequestSent = false; 
                    shouldSend = true; 
                    DEBUG_LOG(">> MISSION DONE: Target reached.");
                }
                else {
                    shouldSend = false; 
                }
            }
        }

        // ---------------------------------------------------------
        // 2. MANUELLE STEUERUNG (Nur wenn keine Mission aktiv ist)
        // ---------------------------------------------------------
        if (!skillActive || currentSkill != 20) {
            if (monitorDirection || holdLine) {
                taskSteer = skillSteer;
                taskSpeed = input_EspNowSpeed;
            } 
            else {
                taskSteer = input_EspNowSteer;
                taskSpeed = input_EspNowSpeed;
            }

            if (lastJoystickCommandTime > 0 && (millis() - lastJoystickCommandTime > JOYSTICK_TIMEOUT)) {
                if (!holdLine) taskSteer = 0;
                taskSpeed = 0;
            }

            if (!digitalRead(BUTTON1_PIN)) { taskSpeed = 200; taskSteer = 0; } 
            else if (!digitalRead(BUTTON2_PIN)) { taskSpeed = -200; taskSteer = 0; }

            if (skill8SafetyActive) { taskSteer = 0; taskSpeed = 0; }
        }

        // ---------------------------------------------------------
        // 3. DATEN-ÜBERTRAGUNG (Nur wenn im Normalmodus)
        // ---------------------------------------------------------
        if (shouldSend) {
            if (!skillActive || currentSkill != 20) {
                global_cmdCode = 0;
                Send(taskSteer, taskSpeed); 
            }
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(30));
    }
} // <--- Diese schließende Klammer hat gefehlt oder war falsch platziert!

void core1WiFiTask(void *parameter) {
    // AP wurde bereits im setup() gestartet. Hier starten wir nur die Server-Dienste.
    DEBUG_LOG("[WiFiTask] Server-Dienste gestartet.");

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.on("/", HTTP_GET, handleWebpage);
    server.on("/resetDevice", HTTP_GET, resetDevice);
    server.on("/reset_wifi", HTTP_GET, handleResetWifi);

    server.onNotFound([](AsyncWebServerRequest *request) {
        if (WiFi.getMode() & WIFI_MODE_AP) {
            handleWebpage(request); 
        } else {
            request->send(404);
        }
    });

    server.on("/setupWiFi", HTTP_GET, [](AsyncWebServerRequest *request){
        String ssid, password;
        if (request->hasParam("ssid") && request->hasParam("password")) {
            ssid = request->getParam("ssid")->value();
            password = request->getParam("password")->value();
            if (ENABLE_DEBUG_SERIAL) {
                debugPrintln("Received new WiFi credentials via AP:");
                debugPrintln(ssid);
            }
            writeWiFiCredentialsToEEPROM(ssid.c_str(), password.c_str());
            request->send(200, "text/plain", "WiFi credentials received. Device is restarting...");
            logToWebpage("WiFi credentials saved. Restarting device to connect...");
            shouldRestart = true;
        } else {
            request->send(400, "text/plain", "Missing SSID or password.");
        }
    });

    server.on("/command", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("cmd") && request->hasParam("peer")) {
            request->send(200, "text/plain", "Command received");
        } else {
            request->send(400, "text/plain", "Invalid request.");
        }
    });

    server.begin(); 

    // 2. Die optimierte Endlosschleife
    while (true) {
        unsigned long now = millis();

        // --- NEU: LED BLINKEN (aus loop verschoben) ---
        digitalWrite(LED_BUILTIN, (now % 2000) < 1000);

        // --- NEU: AP-TIMEOUT LOGIK (aus loop verschoben) ---
        // Wenn mit Haus-WLAN verbunden, schalte eigenen AP nach 5 Min aus
        if (WiFi.status() == WL_CONNECTED && !apShutoffInitiated) {
            if (now - apStartTime >= 300000) { 
                WiFi.softAPdisconnect(true);
                apShutoffInitiated = true;
                
                // Kanal 6 fixieren für ESP-NOW Stabilität
                esp_wifi_set_promiscuous(true);
                esp_wifi_set_channel(ESPNOW_FIXED_CHANNEL, WIFI_SECOND_CHAN_NONE);
                esp_wifi_set_promiscuous(false);

                DEBUG_LOG("AP shut off - Channel locked to 6.");
            }
        }

        // --- NEU: RECONNECT LOGIK (aus loop verschoben) ---
        if (WiFi.status() != WL_CONNECTED && apShutoffInitiated) {
            setupWiFiAP();
            apShutoffInitiated = false;
        }

        // --- DNS & WEBSOCKET (dein originaler Code) ---
        if (WiFi.getMode() & WIFI_MODE_AP) {
            dnsServer.processNextRequest(); 
        }
        
        ws.cleanupClients();  
        vTaskDelay(100 / portTICK_PERIOD_MS); 
    }
}

// ====================================================================================
// ===== CHANGE START: Updated WebSocket Event Handler to process joystick and command 14 data =====
// ====================================================================================
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        DEBUG_LOG("WebSocket client connected");
        client->text("Welcome to ESP32 WebSocket!");
        
        // Send settings including API Key
        DynamicJsonDocument doc(1024);
        doc["type"] = "settings";
        doc["apiKey"] = currentSettings.apiKey;
        String output;
        serializeJson(doc, output);
        client->text(output);

    } else if (type == WS_EVT_DISCONNECT) {
        DEBUG_LOG("WebSocket client disconnected");
    } else if (type == WS_EVT_DATA) {
        // Fixed-size buffer to prevent stack overflow (was: char message[len + 1])
        constexpr size_t MAX_WS_MSG_SIZE = 512;
        char message[MAX_WS_MSG_SIZE];
        size_t copyLen = (len < MAX_WS_MSG_SIZE - 1) ? len : MAX_WS_MSG_SIZE - 1;
        memcpy(message, data, copyLen);
        message[copyLen] = '\0';

        if (len >= MAX_WS_MSG_SIZE) {
            debugPrintln("WebSocket message truncated (too large)");
        }
        
        // *** TASK 1.2: static entfernt (Memory Corruption Fix) ***
        DynamicJsonDocument doc(512); 
        DeserializationError error = deserializeJson(doc, message);
        
        if (error) {
            debugPrint(F("deserializeJson() failed: "));
            debugPrintln(error.c_str());
            return;
        }

        if (!doc["type"].isNull()) {
            const char* msgType = doc["type"];

            // --- SAVE API KEY ---
            if (strcmp(msgType, "saveApiKey") == 0) {
                 if (doc.containsKey("apiKey")) {
                     String newKey = doc["apiKey"].as<String>();
                     if (newKey.length() < 64) {
                         strlcpy(currentSettings.apiKey, newKey.c_str(), sizeof(currentSettings.apiKey));
                         settingsNeedSave = true; // Async speichern im eepromSaveTask
                         client->text("LOG: API Key wird gespeichert...");
                     } else {
                         client->text("LOG: Error - API Key too long.");
                     }
                 }
                 return;
            }

            // --- 1. BLOCK: KOMMANDOS ---
            if (strcmp(msgType, "command") == 0) {
                JsonObject dataObj = doc["data"];
                uint16_t rectangleCode = dataObj["code"] | 0; 

                // --- NEU: Live Log Toggle (Code 998) ---
                if (rectangleCode == 998) {
                    int val = dataObj["value"] | 0;
                    webLogActive = (val == 1);
                    if (webLogActive) {
                        client->text("LOG: --- Live Log ACTIVATED ---");
                        webLogBuffer = ""; // Puffer leeren beim Start
                    } else {
                        client->text("LOG: --- Live Log DEACTIVATED ---");
                    }
                    return; // Wichtig: Hier abbrechen
                }

                // WEB-SERIAL INTERFACE (CODE 999)
                if (rectangleCode == 999) {
                    String webCmd = dataObj["value"].as<String>();
                    webCmd.trim();
                    
                    // Wandelt Leerzeichen in Kommas um für einheitliches Parsing
                    webCmd.replace(' ', ','); 

                    bool isValid = false;

                    if (webCmd.indexOf(',') != -1) {
                        int commaIndex = webCmd.indexOf(',');
                        String cmdPart = webCmd.substring(0, commaIndex);
                        String paramPart = webCmd.substring(commaIndex + 1);
                        
                        cmdPart.trim();
                        paramPart.trim();

                        int cmdInt = cmdPart.toInt(); 

                        if (cmdInt == 16 && isValidInteger(paramPart)) {
                            triggerSkill(16, paramPart.toInt());
                            isValid = true;
                        }
                        else if (cmdInt == 17 && isValidInteger(paramPart)) {
                            triggerSkill(17, paramPart.toInt());
                            isValid = true;
                        }
                        else if (cmdInt == 7 && paramPart.indexOf(',') != -1) {
                            int secondComma = paramPart.indexOf(',');
                            String sSpeed = paramPart.substring(0, secondComma);
                            String sSteer = paramPart.substring(secondComma + 1);
                            
                            if (isValidInteger(sSpeed) && isValidInteger(sSteer)) {
                                triggerSkill(7, 0, sSteer.toInt(), sSpeed.toInt());
                                isValid = true;
                            }
                        }
                        // FIX: Erkennt Code 2 oder 20 und verarbeitet bis zu 6 Argumente
                        else if ((cmdInt == 20 || cmdInt == 2)) {
                            int args[6] = {0, 0, 0, 0, 20, 20}; // Standard 20% Accel/Brake falls nicht angegeben
                            int argCount = 0;
                            String currentString = paramPart;

                            while (currentString.length() > 0 && argCount < 6) {
                                int commaPos = currentString.indexOf(',');
                                String valStr = (commaPos != -1) ? currentString.substring(0, commaPos) : currentString;
                                valStr.trim();
                                if (isValidInteger(valStr)) args[argCount++] = valStr.toInt();
                                currentString = (commaPos != -1) ? currentString.substring(commaPos + 1) : "";
                            }
                            
                            if (argCount >= 2) {
                                // Mappe Parameter: L, R, SpeedL, SpeedR, Accel%, Brake%
                                triggerSkill(20, args[0], args[1], args[2], args[3], args[4], args[5]);
                                isValid = true;
                            }
                        }
                        else if (isValidInteger(cmdPart) && isValidInteger(paramPart)) {
                            Send(cmdInt, paramPart.toInt());
                            isValid = true;
                        }
                    }
                    else if (webCmd.length() > 0 && isValidInteger(webCmd)) {
                        triggerSkill(webCmd.toInt());
                        isValid = true;
                    }

                    if (isValid) client->text("CMD: 999 Val:" + webCmd + " [OK]");
                    else client->text("CMD: 999 Val:" + webCmd + " [INVALID]");
                    return; 
                }

                // REGULÄRE SKILLS
                if (rectangleCode == 7) {
                    triggerSkill(7, 0, dataObj["steer"] | 0, dataObj["speed"] | 0);
                }
                else if (rectangleCode == 10) { 
                    triggerSkill(10, dataObj["working"] | 0, dataObj["stage2"] | 0, dataObj["max"] | 0, dataObj["reset"] | 0); 
                }
                else if (rectangleCode == 11) { 
                    triggerSkill(11, dataObj["value"] | 0);
                }
                else if (rectangleCode == 12 || rectangleCode == 13) { 
                    triggerSkill(rectangleCode, dataObj["shutdown"] | 0, dataObj["safety"] | 0, dataObj["high"] | 0, dataObj["low_temp"] | 0, dataObj["safety_temp"] | 0, dataObj["high_temp"] | 0);
                }
                else if (rectangleCode == 14) { 
                    triggerSkill(14, dataObj["angle"] | 45, dataObj["timeout"] | 750);
                }
                else if (rectangleCode == 15) { 
                    triggerSkill(15, dataObj["p1"] | 0, dataObj["p2"] | 0);
                }
                else if (rectangleCode == 19) {
                    // Format: {"type":"command", "data":{"code":19, "cmd":0, "acc":20, "brk":20}}
                    triggerSkill(19, dataObj["cmd"] | 0, dataObj["acc"] | 20, dataObj["brk"] | 20);
                }
                else if (rectangleCode == 20) { 
                    // Format: {"type":"command", "data":{"code":20, "L":50, "R":50}}
                    triggerSkill(20, dataObj["L"] | 0, dataObj["R"] | 0);
                }
                else if (rectangleCode >= 1 && rectangleCode <= 9) {
                    triggerSkill(rectangleCode, (int16_t)(dataObj["value"] | 0)); 
                }
                else {
                    client->text("ERR: Unknown Code");
                    return;
                }

                // Echo für alle regulären Skills
                String echo = "CMD: " + String(rectangleCode);
                if (!dataObj["value"].isNull()) echo += " Val:" + dataObj["value"].as<String>();
                client->text(echo + " [OK]"); 
            } 

            // --- 2. BLOCK: JOYSTICK (Sauber getrennt durch else if) ---
            else if (strcmp(msgType, "joystick") == 0) {
                if (!doc["steer"].isNull() && !doc["speed"].isNull()) {
                    
                    // --- SAFETY FIX: Sofortiger Reset auf Manuellen Modus ---
                    if (skillActive || global_cmdCode != 0) {
                        skillActive = false;       // Beendet laufende Skills (z.B. Skill 20)
                        currentSkill = 0;
                        currentCase = 0;
                        global_cmdCode = 0;        // WICHTIG: Setzt Hoverboard auf "Speed-Mode" (0) zurück
                        DEBUG_LOG("Joystick Moved -> Forced Manual Mode (Cmd 0)");
                    }
                    // -------------------------------------------------------

                    // *** TASK 2.1: Limitierung (Constrain) für Web-Joystick ***
                    int16_t rawSteer = doc["steer"].as<int16_t>();
                    int16_t rawSpeed = doc["speed"].as<int16_t>();
                    
                    input_EspNowSteer = constrain(rawSteer, -currentSettings.currentMaxSteer, currentSettings.currentMaxSteer);
                    input_EspNowSpeed = constrain(rawSpeed, -currentSettings.currentMaxSpeed, currentSettings.currentMaxSpeed);
                    input_JoySteer = input_EspNowSteer;
                    input_JoySpeed = input_EspNowSpeed;
                    lastJoystickCommandTime = millis();
                    if (ENABLE_DEBUG_SERIAL) {
                        debugPrintf("Joystick: Steer %d, Speed %d\n", input_EspNowSteer, input_EspNowSpeed);
                    }
            }
            }
            else {
                DEBUG_PRINTF("Unknown WS type: %s\n", msgType);
            }
        } 
    } 
}
// ====================================================================================
// ===== CHANGE END: End of updated WebSocket Event Handler ===========================
// ====================================================================================


void espNowTask(void *pvParameters) {
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(onDataReceive);

  while (true) {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// ========================================================
//          Wi-Fi Connection
// ========================================================
void connectToWiFi(const char* ssid, const char* password) {
    int maxRetries = 3;
    int retryCount = 0;
    unsigned long retryDelay = 5000;
    bool connected = false;

    while (retryCount < maxRetries && !connected) {
        if (ENABLE_DEBUG_SERIAL) {
          debugPrint("Connecting to ");
          debugPrintln(ssid);
        }
        logToWebpage("Connecting to " + String(ssid));

        // Wieder zurück auf das 1er Subnetz!
        IPAddress sta_local_IP(192, 168, 1, 123);
        IPAddress sta_gateway(192, 168, 1, 1);
        IPAddress sta_subnet(255, 255, 255, 0);
        WiFi.config(sta_local_IP, sta_gateway, sta_subnet); // Das setzt die Haus-IP fest

        WiFi.begin(ssid, password);
        unsigned long startAttemptTime = millis();

        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
            vTaskDelay(500 / portTICK_PERIOD_MS);
            if (ENABLE_DEBUG_SERIAL) {
              debugPrint(".");
            }
        }


        if (WiFi.status() == WL_CONNECTED) {
            if (ENABLE_DEBUG_SERIAL) {
              debugPrintln("Connected successfully!");
              debugPrint("IP address: ");
              debugPrintln(WiFi.localIP());
            }
            logToWebpage("Connected successfully!");
            logToWebpage("IP address: " + WiFi.localIP().toString());
            isWiFiConnected = true;
            apStartTime = millis();
            connected = true;
        } else {
            retryCount++;
            debugPrintln("Failed to connect. Retrying...");
            logToWebpage("Failed to connect. Retrying... (" + String(retryCount) + "/" + String(maxRetries) + ")");
            delay(retryDelay);
        }
    }

    if (!connected) {
        debugPrintln("Failed to connect after multiple attempts.");
        logToWebpage("Failed to connect after multiple attempts.");
    }
}


// ========================================================
// WebSocket Log Function
// ========================================================
// This function sends log messages to all connected WebSocket clients.
// It is used to mirror the ESP32 serial monitor output to the webpage.

void logToWebpage(const String& message) {
  ws.textAll(message);  // Send the log message to all connected WebSocket clients
}


// Add logic here for what should happen when the resetDevice function is triggered
void resetDevice(AsyncWebServerRequest *request) {
    if (ENABLE_DEBUG_SERIAL) {
      debugPrintln("Resetting device...");
    }
    logToWebpage("Resetting device...");
    request->send(200, "text/plain", "Device is restarting.");
    // *** TASK 1.3: Blockierenden Code durch Flag ersetzt ***
    shouldRestart = true;
}

// Add this new function to handle the WiFi reset request
void handleResetWifi(AsyncWebServerRequest *request) {
    if (ENABLE_DEBUG_SERIAL) {
      debugPrintln("Resetting WiFi credentials in EEPROM...");
    }
    logToWebpage("Resetting WiFi credentials in EEPROM...");
    // Write empty strings to the credential memory locations
    writeWiFiCredentialsToEEPROM("", "");
    request->send(200, "text/plain", "WiFi credentials reset. Please restart the device.");
    logToWebpage("WiFi credentials have been reset on the device. A restart is required to apply the changes.");
}


// ====================================================================================
// ===== AFTER: Optimierte Receive Funktion mit Flag-Check & Erweitertem Debug ========
// ====================================================================================
void Receive() {
  while (HoverSerial.available()) {
    incomingByte = HoverSerial.read();
    bufStartFrame = ((uint16_t)(incomingByte) << 8) | incomingBytePrev;

    if (bufStartFrame == START_FRAME) { // Benutze das definierte START_FRAME (0xABCD)
      p = (byte *)&NewFeedback;
      *p++ = incomingBytePrev;
      *p++ = incomingByte;
      idx = 2;
    } 
    else if (idx >= 2 && idx < sizeof(SerialFeedback)) {
      *p++ = incomingByte;
      idx++;
    }

    if (idx == sizeof(SerialFeedback)) {
      // Prüfsumme berechnen (XOR-Kette synchron zur Hoverboard-Firmware)
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

        // 1. BUSY FLAG EXTRAHIEREN (Bit 8: Meldet der STM32 'dist_mode_active'?)
        hoverboardIsBusy = (Feedback.cmdLed & 0x0100) != 0;

        // 2. DATEN-INTERPRETATION: 
        // Wenn Busy (Mission): cmd1/cmd2 sind gefahrene Ticks
        // Wenn Idle: cmd1/cmd2 sind die aktuell gemessenen Geschwindigkeiten/PWM
        if (hoverboardIsBusy) {
          feedbackDistL = Feedback.cmd1;
          feedbackDistR = Feedback.cmd2;
        } else {
          // Nur nullen, wenn keine Mission aktiv ist
          if (currentSkill != 20) {
            feedbackDistL = 0;
            feedbackDistR = 0;
          }
        }
      }
      idx = 0; // Reset für nächstes Paket
    }
    incomingBytePrev = incomingByte; 
  }

  // Debug Ausgabe: Erweitert um den Status der Busy-Flag und die Ticks
  if (PRINT_FEEDBACK_BLOCK) {
    debugPrintf("FB: V=%d | L_Val=%d | R_Val=%d | T=%d | BUSY=%s\n", 
                Feedback.batVoltage, 
                Feedback.cmd1, 
                Feedback.cmd2, 
                Feedback.boardTemp,
                hoverboardIsBusy ? "YES" : "NO");
  }
}

// ########################## LOOP ##########################

void loop(void)
{
    // 1. System-Neustart (falls über Web getriggert)
    if (shouldRestart) {
        delay(100);
        ESP.restart();
    }

    // 2. Daten vom Hoverboard empfangen
    Receive();

    // 3. Sicherheits-Checks & Status-Berechnung
    // Batteriespannung korrigieren (expliziter Cast für Typensicherheit)
    corrected_batVoltage = (int16_t)(Feedback.batVoltage * currentSettings.driveBatteryFactor);

    // Board-Temperatur mit Offset berechnen
    float rectangleCorrectedBoardTemp = Feedback.boardTemp + 37.6f;

    // Systemstatus prüfen (Spannung/Temp Limits) - Phase 3: Removed unused params
    handleSystemStatus(corrected_batVoltage, rectangleCorrectedBoardTemp);

    // 4. ESP-NOW & Peer Management
    managePeers();

    // 5. Konsolen-Eingaben (USB) verarbeiten
    handleSerialInput();

    // 6. & 7. Fahrlogik & Hoverboard-Send
    // Diese Logik wurde vollständig in die 'controlLogicTask' auf Core 1 ausgelagert.
    // Das garantiert ein stabiles 30ms Sende-Intervall, unabhängig von der CPU-Last.
    
    // Wir aktualisieren hier nur noch die Variablen für die Web-Anzeige
    // 8. Web Log Timer (Log-Daten an Website schicken alle 500ms)
        if (webLogActive && (millis() - lastWebLogSend > 500)) {        lastWebLogSend = millis();
        if (webLogBuffer.length() > 0) {
            ws.textAll("LOG:" + webLogBuffer); 
            webLogBuffer = ""; 
        }
    }

    // 9. Mähwerk-Logik (Blade)
    runBladeLogic();

    ArduinoOTA.handle(); // Prüft ständig, ob ein neues Update über WLAN reinkommt

    // 10. Kurze Pause für Hintergrundprozesse
    vTaskDelay(10 / portTICK_PERIOD_MS);
}

void mpuReadTask(void *parameter) {
  DEBUG_LOG("MPU Reading Task (Core 0) started.");
  
  for (;;) {
    // Erhöhter Timeout für Semaphore, um auf ISR zu warten
    if (xSemaphoreTake(mpuSemaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
      if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        
        fifoCount = mpu.getFIFOCount();
        
        // Wenn der Puffer zu voll wird (> 500 Bytes), ist er "veraltet". 
        // Wir löschen ihn komplett für frische Daten.
        if (fifoCount == 1024 || fifoCount > 500) {
          mpu.resetFIFO();
        } 
        else {
          // Wir lesen nur das aktuellste Paket und verwerfen den Rest im Puffer schnell,
          // um Latenz zu vermeiden (Real-Time Orientierung).
          while (fifoCount >= packetSize) {
            mpu.getFIFOBytes(fifoBuffer, packetSize);
            fifoCount -= packetSize;
          }

          // Nur das letzte (aktuellste) Paket verarbeiten
          mpu.dmpGetQuaternion(&q, fifoBuffer);
          mpu.dmpGetGravity(&gravity, &q);
          mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

          yaw   = 2.0f * ((ypr[0] * 180 / M_PI) - yawOffset);
          pitch = (ypr[1] * 180 / M_PI) - pitchOffset;
          roll  = (ypr[2] * 180 / M_PI) - rollOffset;

          if (yaw > 180)  yaw -= 360;
          if (yaw < -180) yaw += 360;

          if (skill8Active) {
              monitorSkill8Angles(pitch, roll);
              if (monitorDirection) monitorDirectionChange(yaw);
              if (holdLine && (millis() - case3StartTime >= 500)) holdTheLine(yaw);
          }
        }
        xSemaphoreGive(i2cMutex);
      }
    }
    // Task-Delay reduzieren für schnellere Reaktion (Core 0 hat Zeit)
    vTaskDelay(pdMS_TO_TICKS(5)); 
  } 
}


// ========================================================
void handleSystemStatus(int16_t battery, int16_t temp) {

    static unsigned long driveHighVoltTime = 0;
    static unsigned long driveLowVoltTime = 0;
    static unsigned long driveTempErrorTime = 0;
    static unsigned long bladeVoltErrorTime = 0;

    unsigned long now = millis();

    // 1. DRIVE SYSTEM MONITORING (Hoverboard)
    if (battery > 0) {
        // A. Überspannung Drive
        if (battery > currentSettings.driveHighVoltage) {
            if (driveHighVoltTime == 0) driveHighVoltTime = now;
            if (now - driveHighVoltTime > SAFETY_TRIGGER_DELAY && !isHighVoltageShutdown) {
                isHighVoltageShutdown = true;
                debugPrintln("!!! SHUTDOWN: Drive High Voltage (Persistent)!");
                triggerSkill(8, 20);
            }
        } else { driveHighVoltTime = 0; }

        // B. Kritische Unterspannung Drive
        if (battery < currentSettings.driveMinShutdownVoltage) {
            if (driveLowVoltTime == 0) driveLowVoltTime = now;
            if (now - driveLowVoltTime > SAFETY_TRIGGER_DELAY) {
                Send(0, 0);
                debugPrintln("!!! STOP: Drive Battery Critically Low!");
            }
        } else { driveLowVoltTime = 0; }

        // C. Safety Mode (Warnung/Drosselung - ohne Delay für sofortige Reaktion)
        if (battery < currentSettings.driveSafetyModeVoltage && battery >= currentSettings.driveMinShutdownVoltage) {
            if (!isInSafetyMode) {
                isInSafetyMode = true;
                originalMaxSpeed = currentSettings.currentMaxSpeed;
                currentSettings.currentMaxSpeed /= 2;
                triggerSkill(6, -1);
                debugPrintln("!! SAFETY MODE: Low Drive Battery - Blade OFF");
            }
        } else if (battery >= currentSettings.driveSafetyModeVoltage && isInSafetyMode) {
            // Automatischer Reset des Safety Modes, wenn Spannung wieder OK
            isInSafetyMode = false;
            currentSettings.currentMaxSpeed = originalMaxSpeed;
            debugPrintln("SYSTEM: Drive Voltage restored. Normal speed.");
        }
    }

    // 2. DRIVE TEMPERATURE CHECK
    if (temp > -100) { 
        if (temp > currentSettings.driveHighTemp || (temp != 0 && temp < currentSettings.driveEmergencyLowTemp)) {
            if (driveTempErrorTime == 0) driveTempErrorTime = now;
            if (now - driveTempErrorTime > SAFETY_TRIGGER_DELAY) {
                debugPrintf("!!! SHUTDOWN: Drive Temp Error Persistent: %d\n", temp);
                triggerSkill(8, 20);
            }
        } else { driveTempErrorTime = 0; }
    }

    // 3. BLADE SYSTEM MONITORING (Zusatz-Akku) - Integriert in handleSystemStatus
    if (global_blade_voltage_mv > 5000) { 
        int32_t minLimitMv = (int32_t)currentSettings.bladeMinShutdownVoltage * 10;
        int32_t maxLimitMv = (int32_t)currentSettings.bladeHighVoltage * 10;

        if (global_blade_voltage_mv < minLimitMv || global_blade_voltage_mv > maxLimitMv) {
            if (bladeVoltErrorTime == 0) bladeVoltErrorTime = now;
            if (now - bladeVoltErrorTime > SAFETY_TRIGGER_DELAY && currentBladeState != BLADE_OFF) {
                debugPrintf("!!! BLADE STOP: Voltage %d mV Persistent Error! Relay OFF.\n", global_blade_voltage_mv);
                triggerSkill(6, -1);
            }
        } else { bladeVoltErrorTime = 0; }
    }
}

void resetHorizon() {
    yawOffset = ypr[0] * 180 / M_PI;
    pitchOffset = ypr[1] * 180 / M_PI;
    rollOffset = ypr[2] * 180 / M_PI;
    // ====================================================================================
    // ===== CHANGE START: Added capsize timer reset to horizon calibration =============
    // ====================================================================================
    capsizeDetectTime = 0; // Also reset the capsize detection timer to prevent false triggers after reset
    // ====================================================================================
    // ===== CHANGE END: End of timer reset =============================================
    // ====================================================================================
    DEBUG_LOG("Horizon reset: All angles set to 0°.");
}

void resetAll() {
    currentCase = 0;
    monitorDirection = false;
    holdLine = false;
    turnDirection = 0;
    skillActive = false;
    currentSkill = 0;
    skillSteer = 0;
    input_EspNowSpeed = 0;
    input_EspNowSteer = 0;
    capsizeDetectTime = 0;
    lastError = 0.0f;
    lastPdTime = 0;

    // Blade Unit auf Stand-by (1500)
    digitalWrite(BLADE_UNIT_PIN, HIGH); 
    currentBladeState = BLADE_WORK;
    current_blade_pwm = BLADE_ZERO_US;

    DEBUG_LOG("System reset: All states cleared.");
}

// ====================================================================================
// ===== PHASE 3 REFACTORING: Helper Functions ========================================
// ====================================================================================

/**
 * @brief Normalizes an angle to the range [-180, 180] degrees.
 * @param angle The input angle in degrees
 * @return The normalized angle in range [-180, 180]
 */
inline float normalizeAngle(float angle) {
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

/**
 * @brief Calculates the shortest angular difference between two angles.
 * @param from Start angle in degrees
 * @param to Target angle in degrees
 * @return The signed difference in range [-180, 180]
 */
inline float angleDifference(float from, float to) {
    float diff = to - from;
    return normalizeAngle(diff);
}

/**
 * @brief PD-Controller calculation for steering correction.
 * Extracted from monitorDirectionChange and holdTheLine (Phase 3.1).
 *
 * @param error The current error value (deviation from target)
 * @param Kp Proportional gain
 * @param Kd Derivative gain
 * @param maxSteer Maximum steering output limit
 * @return The calculated steering value, constrained to [-maxSteer, maxSteer]
 */
int16_t calculatePDSteer(float error, float Kp, float Kd, int16_t maxSteer) {
    unsigned long now = millis();
    unsigned long timeDelta = now - lastPdTime;

    // Prevent division by zero (thread-safe with < 1)
    if (lastPdTime == 0 || timeDelta < 1) {
        timeDelta = 1;
    }

    // Calculate derivative (rate of error change)
    float derivative = (error - lastError) / (float)timeDelta;

    // PD output
    float pdOutput = (Kp * error) + (Kd * derivative);

    // Store state for next iteration
    lastError = error;
    lastPdTime = now;

    // Constrain and return
    return constrain((int16_t)pdOutput, -maxSteer, maxSteer);
}

// ====================================================================================

// ÄNDERN Sie die Funktion "monitorDirectionChange"
void monitorDirectionChange(float yaw) {
    float rectangleError = fmod(targetAngle - yaw + 540, 360) - 180;

    // Fix für 180-Grad-Drehung
    if (turnDirection == 1 && rectangleError < -90) rectangleError += 360;
    else if (turnDirection == -1 && rectangleError > 90) rectangleError -= 360;

    // PD-Controller (Phase 3.1: Extracted to helper function)
    skillSteer = calculatePDSteer(rectangleError, currentSettings.Kp, currentSettings.Kd, currentSettings.currentMaxSteer);

    if (ENABLE_DEBUG_SERIAL && (millis() - lastSerialUpdate >= 500)) {
        lastSerialUpdate = millis();
        DEBUG_PRINTF("Yaw: %.1f, Err: %.1f, Steer: %d\n", yaw, rectangleError, skillSteer);
    }

    if (abs(rectangleError) < PD_ERROR_THRESHOLD) {
        DEBUG_LOG("Target angle reached.");
        resetAll();
    }
}

// ÄNDERN Sie die Funktion "holdTheLine"
void holdTheLine(float yaw) {
    // Fehler ist die Abweichung vom Startwinkel (Phase 3.1: Using helper)
    float rectangleError = normalizeAngle(startYaw - yaw);

    // PD-Controller (Phase 3.1: Extracted to helper function)
    skillSteer = calculatePDSteer(rectangleError, currentSettings.Kp, currentSettings.Kd, currentSettings.currentMaxSteer);

    if (ENABLE_DEBUG_SERIAL && (millis() - lastSerialUpdate >= 500)) {
        lastSerialUpdate = millis();
        DEBUG_PRINTF("HoldLine | Yaw: %.1f, Dev: %.1f, Steer: %d\n", yaw, -rectangleError, skillSteer);
    }
}



void statusTask(void *pvParameters) {
    // Statische Variable für die Glättung
    static float smoothed_blade_mv = 0;

    for (;;) {
        // Lokale Kopie des Feedbacks mit Mutex-Schutz
        SerialFeedback safeFeedback;
        if (xSemaphoreTake(feedbackMutex, (TickType_t)5) == pdTRUE) {
            memcpy(&safeFeedback, &Feedback, sizeof(SerialFeedback));
            xSemaphoreGive(feedbackMutex);
        }
        // =========================================================
        // 1. SCHNELLE MESSUNG (Alle 50ms)
        // =========================================================
        
        // 1a. Spannung lesen (Millivolt am Pin 1)
        uint32_t raw_pin_mv = analogReadMilliVolts(BLADE_BATTERY_PIN);

        // 1b. Mit Faktor verrechnen
        float calculated_mv = (float)raw_pin_mv * currentSettings.bladeBatteryFactor;

        // 1c. INTELLIGENTE GLÄTTUNG (Optimiert für Stabilität)
        
        // FALL A: START-UP (Schnell)
        // Wenn der gespeicherte Wert noch unter 5V ist (System startet gerade),
        // übernehmen wir den neuen Wert sofort. Keine Wartezeit beim Einschalten!
        if (smoothed_blade_mv < 5000) {
             smoothed_blade_mv = calculated_mv;
        } 
        // FALL B: BETRIEB (Stabil & Träge)
        // Hier nutzen wir den Filter. 95% Alter Wert, 5% Neuer Wert.
        // Das bügelt Schwankungen durch Motorenstarts komplett glatt.
        else {
             smoothed_blade_mv = (smoothed_blade_mv * 0.95) + (calculated_mv * 0.05);
        }

        // 1d. Global speichern
        global_blade_voltage_mv = (int32_t)smoothed_blade_mv;

        /*
        // --- DEBUG AUSGABE (Alle 2 Sekunden im Monitor sichtbar) ---
        static unsigned long lastDebugPrint = 0;
        if (millis() - lastDebugPrint > 2000 && ENABLE_DEBUG_SERIAL) {
             lastDebugPrint = millis();
             debugPrintf("[StatusTask] Pin1 Raw: %d mV | Factor: %.2f | Calc Bat: %d mV\n", 
                           raw_pin_mv, currentSettings.bladeBatteryFactor, global_blade_voltage_mv);
        }
        */
        // -----------------------------------------------------------

        // Warte das Sende-Intervall ab
        vTaskDelay(statusSendInterval / portTICK_PERIOD_MS);  
        
        if (ws.count() > 0) {
            DynamicJsonDocument doc(512);
            doc["type"] = "status";
            doc["estop"] = skill8SafetyActive; // Sendet true oder false an die Webseite
            
            // --- DRIVE BATTERY (Hoverboard) ---
            if (corrected_batVoltage < 500) { 
                doc["battery"] = nullptr; 
            } else { 
                doc["battery"] = corrected_batVoltage; 
            }
            
            // --- DRIVE TEMP (Hoverboard) ---
            // Feedback ist z.B. 300 (30.0°C). Webseite teilt durch 100.
            // Wir senden mal 10 -> 3000 / 100 = 30.00
            if (safeFeedback.boardTemp == 0) { 
                doc["temp"] = nullptr; 
            } else { 
                doc["temp"] = safeFeedback.boardTemp * 10; 
            }
            
            // --- BLADE BATTERY (Zusatzakku) ---
            // Wir haben den Schwellwert auf 1000mV (1V) gesenkt, damit du was siehst.
            if (global_blade_voltage_mv < 1000) {
                doc["blade_battery"] = nullptr;
            } else {
                // Wir senden Centivolt an die Webseite
                // Bsp: 16348 mV / 10 = 1634. Webseite rechnet 1634 / 100 = 16.34 V
                doc["blade_battery"] = global_blade_voltage_mv / 10;
            }
            
            // Dummys für Sensoren, die wir (noch) nicht haben
            doc["battery_temp"] = 0;
            doc["blade_temp"] = 0; // Das erzeugt das "NN" oder "--" für Blade Temp

            // Sende Blade-Status (hilfreich für die Web-UI)
            doc["blade_state"] = (currentBladeState == BLADE_OFF) ? "off" : "work";
            String bladeHeightStr = "unknown";
            if (currentBladeHeight == BLADE_HEIGHT_DOWN) bladeHeightStr = "down";
            else if (currentBladeHeight == BLADE_HEIGHT_UP) bladeHeightStr = "up";
            doc["blade_height"] = bladeHeightStr;

            // --- SPEED CALCULATION (Moved to StatusTask to protect Serial Communication) ---
            // Calculate speed in km/h based on average RPM of both wheels
            
            // FIX: Right wheel is mounted mirrored, so its feedback is inverted (negative for forward)
            // We invert it here to get positive values for forward driving.
            int16_t speedR_corrected = -safeFeedback.speedR_meas;

            float avg_rpm = (safeFeedback.speedL_meas + speedR_corrected) / 2.0f;
            
            // Formula: kmh = (avg_rpm * 0.95m * 60) / 1000
            current_speed_kmh = (avg_rpm * 0.95f * 60.0f) / 1000.0f;
            
            // Calculate individual speeds
            float speedL_kmh = (Feedback.speedL_meas * 0.95f * 60.0f) / 1000.0f;
            float speedR_kmh = (speedR_corrected * 0.95f * 60.0f) / 1000.0f;

            doc["speed_kmh"] = current_speed_kmh;
            doc["speedL_kmh"] = speedL_kmh;
            doc["speedR_kmh"] = speedR_kmh;

            // Blade mode: off / standby / speed1 / speed2 / web_pwm
            String bladeMode = "off";
            if (currentBladeState == BLADE_OFF) {
                bladeMode = "off";
            } else if (currentBladeState == BLADE_CABLE_RESET) {
                bladeMode = "cable_reset";
            } else if (currentBladeState == BLADE_WORK) {
                if (current_blade_pwm == BLADE_ZERO_US) bladeMode = "standby";
                else if (current_blade_pwm == currentSettings.bladeWorkingSpeed) bladeMode = "speed1";
                else if (current_blade_pwm == currentSettings.bladeStage2Speed) bladeMode = "speed2";
                else bladeMode = "web_pwm";
            }
            doc["blade_mode"] = bladeMode;

            // Skills / Drive modes
            doc["hold_line"] = holdLine;
            doc["turn_active"] = skillActive;
            // Provide detail which turn is active so the Web UI can highlight the correct button
            doc["turn_case"] = currentCase;      // 2 = 90°, 4 = 180°
            doc["turn_direction"] = turnDirection; // -1 = left, 1 = right, 0 = none
            doc["cable_reset"] = (currentBladeState == BLADE_CABLE_RESET);
            // stop_reset is the -20 command (Stop/Reset); we cannot know if it's active but include false for now
            doc["stop_reset"] = false;

            String jsonString;
            serializeJson(doc, jsonString);
            ws.textAll(jsonString); 
        }
    }
}

// Task 5.5: EEPROM Speicher-Task (asynchrones Speichern außerhalb des Web-Threads)
void eepromSaveTask(void *pvParameters) {
    for (;;) {
        if (settingsNeedSave) {
            if (saveSettings()) {
                DEBUG_LOG("Settings saved successfully (async)");
            } else {
                DEBUG_LOG("ERROR: Failed to save settings (async)");
            }
            settingsNeedSave = false;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); // Prüfe alle 100ms
    }
}

extern const char webpage_part1[] PROGMEM;
extern const char webpage_part2[] PROGMEM;

void handleWebpage(AsyncWebServerRequest *request) {
    // Wir senden die Antwort "gestückelt" (Chunked Response)
    AsyncWebServerResponse *response = request->beginChunkedResponse("text/html", [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
        
        // Gesamtlänge beider Teile
        size_t len1 = strlen_P(webpage_part1);
        size_t len2 = strlen_P(webpage_part2);
        size_t totalLen = len1 + len2;

        // Wenn wir alles gesendet haben, gib 0 zurück (Ende)
        if (index >= totalLen) {
            return 0;
        }

        // Berechne, wie viel wir in diesem "Chunk" senden können
        size_t bytesRemaining = totalLen - index;
        size_t bytesToSend = (bytesRemaining < maxLen) ? bytesRemaining : maxLen;

        // Daten kopieren
        for (size_t i = 0; i < bytesToSend; i++) {
            size_t currentPos = index + i;
            if (currentPos < len1) {
                // Wir sind noch im ersten Teil
                buffer[i] = pgm_read_byte(webpage_part1 + currentPos);
            } else {
                // Wir sind im zweiten Teil
                buffer[i] = pgm_read_byte(webpage_part2 + (currentPos - len1));
            }
        }
        return bytesToSend;
    });

    request->send(response);
}