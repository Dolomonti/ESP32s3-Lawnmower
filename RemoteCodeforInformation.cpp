/*
/*
// ====================================================================================
// ===== KEEP ME IN EVERY VERSION OF THIS CODE HERE ON TOP ============================
// ====================================================================================
//
// This is the REMOTE CONTROLLER code, intended for an ESP32 WROOM 32 board.
// To ensure successful compilation, please use the following environment setup:
//
// -- BOARD SETTINGS (for ESP32-WROOM-DA Module) --
// Board: "ESP32-WROOM-DA Module"
// Upload Speed: "921600"
// CPU Frequency: "240MHz (WiFi/BT)"
// Flash Frequency: "80MHz"
// Flash Mode: "QIO"
// Flash Size: "4MB (32Mb)"
// Partition Scheme: "Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)"
// Core Debug Level: "None"
// Arduino Runs On: "Core 1"
// Events Run On: "Core 1"
// Erase All Flash Before Sketch Upload: "Disabled"
//
// -- BOARD MANAGER --
// 1. In Arduino IDE -> Settings -> "Additional boards manager URLs", add:
//    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
// 2. Go to Tools -> Board -> Boards Manager...
// 3. Search for "esp32" and install "esp32 by Espressif Systems" version 2.0.17.
//
// -- REQUIRED LIBRARIES (from your installed list) --
// Use the Arduino Library Manager (Sketch -> Include Library -> Manage Libraries...) to install:
//
// Core Libraries for This Project:
// 1. ArduinoJson by Benoit Blanchon (Version 7.4.2)
// 2. Async TCP by ESP32Async (Version 3.4.7)
// 3. ESP Async WebServer by ESP32Async (Version 3.8.0)
// 4. MPU6050_tockn by tockn (Version 1.5.2)
// 5. PS3 Controller Host by Jeffrey van Pernis (Version 2.0.0)
//
// Other Installed Libraries:
// - NTPClient by Fabrice Weinberg (Version 3.2.1)
// - Servo by Michael Margolis, Arduino (Version 1.2.2)
// - ArduinoSTL by Mike Matera (Version 1.3.3)
// - BluetoothSerial by Henry Abrahamsen (Version 1.1.0)
// - DoubleResetDetector_C by Khoi Hoang (Version 1.8.1)
// - ESP32Servo by Kevin Harrington, John K... (Version 3.0.8)
// - ESP_AT_Lib by Khoi Hoang (Version 1.5.1)
// - EthernetWebServer by Khoi Hoang (Version 2.4.1)
// - EthernetWebServer_STM by Khoi Hoang (Version 1.5.0)
// - Ethernet_Generic by Various (Version 2.8.1)
// - FlashStorage_RTL8720 by Khoi Hoang (Version 1.1.0)
// - FlashStorage_SAMD by Cristian Maglie (Version 1.3.2)
// - FlashStorage_STM32 by Khoi Hoang (Version 1.2.0)
// - FlashStorage_STM32F1 by Khoi Hoang (Version 1.1.0)
// - Functional-Vlpp by Khoi Hoang (Version 1.0.2)
// - NimBLE-Arduino by h2zero (Version 2.3.4)
// - STM32duino LwIP by Adam Dunkels... (Version 2.1.3)
// - STM32duino STM32Ethernet by... (Version 1.4.0)
// - SinricPro_Generic by Boris Jaeger... (Version 2.8.5)
// - UIPEthernet by Norbert Truchsess, Cassy... (Version 2.0.12)
// - USB Host Shield Library 2.0 by Oleg... (Version 1.7.0)
// - WebServer_WT32_ETH0 by Khoi Hoang (Version 1.5.1)
// - WebSockets_Generic by Markus Sattler, Khoi... (Version 2.16.1)
// - WiFi101_Generic by Arduino (Version 1.0.0)
// - WiFiEspAT by Juraj Andrassy (Version 1.5.0)
// - WiFiMulti_Generic by Khoi Hoang (Version 1.2.2)
// - WiFiNINA_Generic by Arduino, Khoi Hoang... (Version 1.8.15-1)
// - WiFiWebServer by Khoi Hoang (Version 1.10.1)
// ====================================================================================
*/
/* RC Car Controller ESP32 WROOM 32 WITH EXTENDED Antenna using Bluetooth PS3 Controller 
this module mac adreess 30:C6:F7:25:FD:F8

receiver module atm is: mac adress 48:27:E2:E9:11:9C
ESP32 S3

 Description:
 Select control mode: with SELECT BUTTON on joystick 
 1: Raw values
 2: Three fields logic
 3: Threshold-based acceleration


 // Structure for ESP-NOW commands
/**
 * ESPNowCommand: Structure used to receive remote commands via ESP-NOW
 * Fields:
 * - steer: Steering value (e.g., 0 for straight, positive for right, negative for left)
 * - speed: Speed value (e.g., 200 for forward, -200 for backward)
 * - reset: Special command for skill or to reset the ESP32
 * Example:
 * To send a forward command:
 * {steer: 0, speed: 200, commandcode: 0}
 * To reset the ESP32:
 * {steer: 0, speed: 0, commandcode: 9}

Available CommandCodes (0-9)
commandCode 0 - Movement Command
commandCode 1 - skill Reset on Receiver
commandCode 2 - 90-degree turn (left or right)
commandCode 3 - Hold position or line-following
commandCode 4 - 180-degree turn (left or right)
commandCode 5 - Stop or hold position
commandCode 6 - Blade (on/off)
commandCode 7 - Set max speed
commandCode 8 - Additional functionality (user-defined)
commandCode 9 - Reset Receiver ESP32

3. Other Button Mappings
Button	Action	Command Sent
Cross (X)	 	System Reset on Receiver.	commandCode 1
Square	 	 Prepare 90-degree turn.	None
Circle	 	Prepare 180-degree turn.	None
Triangle	  Hold straight path (sets steer = 0).	commandCode 3
Start	 	  Reset Receiver, then reset Remote.	commandCode 9
Select	 	 Cycle local control modes.	None
R1	 	   Blade ON command.	commandCode 6
R2	 	   Blade OFF command.	commandCode 6
D-Pad Up/Down	 	 	Adjust speed mode (1=lowest, 4=highest).	None
D-Pad Left/Right	 Execute prepared turn / Adjust steering trim.	commandCode 2/4 or None
4. Unused Buttons
L1:	 	  Configurable: Deadman Switch OR E-Stop. commandCode 5
L2:	 	  reset joystick control mode to mode 1

PS Button: Reserved for controller pairing (handled by Ps3Controller library).

// Analog Sticks:
// Left Stick (Vertical) --> Throttle (Speed Adjustment Forward/Reverse)
// Left Stick (Horizontal) --> Display value only.
// Right Stick (Horizontal) --> Steering (Left/Right)
// Right Stick (Vertical) --> Display value only.
// Left Stick Button --> Cruise control for speed.
// Right Stick Button --> Cruise control for steering.

*/

/*
--- LOCAL JOYSTICK CONTROL MODES (DEPRECATED / Informational) ---
The 'controlMode' variable, previously toggled by the SELECT button, only changes how the joystick's
analog stick values are interpreted locally on this remote. It does not send any mode change command
to the mower/receiver. The logic for modes 2 and 3 was never fully implemented in the joystick
processing section, so the remote effectively always operates in Mode 1.

1. Mode 1: Raw Values (Linear Control)
   - Logic: This is the most straightforward mode. It takes the raw position of the joystick
     (from -128 to 127) and directly scales it to the available speed and steering range.
   - Effect: The response is linear. Pushing the joystick 50% of the way forward gives you 50%
     of the maximum speed. This is the default and current operational mode.

2. Mode 2: Three Fields Logic (Tiered Control)
   - Logic: This mode was intended to divide the joystick's movement into three distinct zones:
     a) A Dead Zone: A small area around the center where no movement is registered.
     b) A Middle Zone: A range of movement resulting in a capped, lower speed for predictable maneuvering.
     c) An Upper Zone: Pushing past the middle zone scales the speed to the absolute maximum.
   - Effect: Creates a tiered speed response, useful for having a predictable "walking pace"
     without needing to hold the joystick perfectly, while still allowing access to full speed.

3. Mode 3: Threshold-based Acceleration (Exponential Curve)
   - Logic: This mode applies a mathematical curve to the joystick input. The ACCELERATION_CURVE
     value of 2 suggests an exponential curve (like output = input^2).
   - Effect: The response is non-linear. Small joystick movements result in very fine-tuned
     adjustments, while larger movements cause rapid acceleration. Excellent for tasks requiring
     high precision at low speeds.
*/

#include <Ps3Controller.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#define MAX_SPEED 800       // Define maximum speed for throttle
#define MAX_STEER 800      // Define maximum steering value
#define TRIM_STEP 50        // Define step size for trim adjustments

// Select control mode here: (1,2 or 3) later with select button on joystick
#define CONTROL_MODE 2      // Default control mode

// Driving direction inversion mode: 1 = Normal, 2 = Inverted
#define DRIVING_DIRECTION 1  // Default driving direction

// --- L1 BUTTON CONFIGURATION ---
// Set to 'true' to use L1 as a Deadman Switch (must be held down to drive).
// Set to 'false' to use L1 as a single-press Emergency Stop button.
const bool L1_IS_DEADMAN_SWITCH = false;

// General threshold to avoid false commands
#define BELOW_THRESHOLD 0.1

// 1: Raw values
#define BELOW_THRESHOLD_1 0.1  // Below threshold percentage for raw values
#define TOP_THRESHOLD 995      // Top threshold for raw values

// 2: Three fields logic (at the moment acts like a 2 field logic)
#define BELOW_THRESHOLD_2 0.1  // Below threshold percentage for three fields logic
#define MIDDLE_ZONE_2 0.2      // Middle zone upper limit as percentage for three fields logic
#define UPPER_LEVEL_2 0.8      // Upper level for the middle zone in three fields logic

// 3: Threshold-based acceleration
#define BELOW_THRESHOLD_3 0.2  // Below threshold percentage for threshold-based acceleration
#define ACCELERATION_CURVE 2   // Curve factor for threshold-based acceleration logic

#define LED_PIN             4  // Built-in LED


// ESP-NOW Configuration
#define ESPNOW_CHANNEL (6)
#define ESPNOW_ENCRYPT false

// ESP-NOW Command Structure
typedef struct __attribute__((packed)) {
    uint16_t start;         // Start frame (0xABCD)
    int16_t steer;          // Steering value
    int16_t speed;          // Speed value
    uint8_t commandCode;    // Command code
    uint16_t checksum;      // Checksum
} ESPNowCommand;

/* Global Variables */
int player = 0;
int battery = 0;
int16_t steer = 0; // Steering value: 0 (straight), positive (right), negative (left).
int16_t speed = 0; // Speed value: positive (forward), negative (backward).
int speedMode = 1; // Default to Mode 1 (low speed)
int steeringTrim = 0; // Steering trim offset
bool isControllerConnected = false; // Track if the controller is connected
bool cruiseControlSpeed = false; // Cruise control state for speed
bool cruiseControlSteer = false; // Cruise control state for steering
bool bladeActive = false;        // NEU: Status für Messer-Toggleint controlMode = CONTROL_MODE; // Set default control mode
int drivingDirection = DRIVING_DIRECTION; // Initialize driving direction
int preparedTurnType = 0; // State for two-step turns: 0=none, 90=90-deg, 180=180-deg
unsigned long lastCommandTime = 0;  // Put with other global variables
volatile bool deliverySuccess = false;
volatile bool deliveryFailed = false;

// bool l1SafetyEnabled is replaced by L1_IS_DEADMAN_SWITCH constant

bool shouldSendJoystickCommand(int16_t speed, int16_t steer) {
    int16_t speedThreshold = MAX_SPEED * BELOW_THRESHOLD;
    int16_t steerThreshold = MAX_STEER * BELOW_THRESHOLD;
    return (abs(speed) > speedThreshold) || (abs(steer) > steerThreshold);
}




// --- NEU: Zwei Empfänger-MACs ---
unsigned long lastSendTime = 0; // Diese Zeile hat gefehlt!
uint8_t receiver1[] = {0x48, 0x27, 0xE2, 0xE9, 0x11, 0x9C}; // AKTIVER Empfänger (Vorheriger, 9C) - Jetzt Standard
uint8_t receiver2[] = {0xDC, 0xDA, 0x0C, 0x52, 0xDE, 0x84}; // Neuer S3 (Backup/Inaktiv)
uint8_t* activeReceiver = receiver1;




// ESP-NOW initialization
void initESPNow() {
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        ESP.restart();
        return;
    }
    
    esp_now_register_send_cb([](const uint8_t *mac_addr, esp_now_send_status_t status) {
        deliverySuccess = (status == ESP_NOW_SEND_SUCCESS);
        deliveryFailed = !deliverySuccess;
    });

    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    peerInfo.channel = ESPNOW_CHANNEL; 
    peerInfo.ifidx = WIFI_IF_STA; 
    peerInfo.encrypt = ESPNOW_ENCRYPT;
    
    memcpy(peerInfo.peer_addr, receiver1, 6);
    esp_now_add_peer(&peerInfo);

    memcpy(peerInfo.peer_addr, receiver2, 6);
    esp_now_add_peer(&peerInfo);

    Serial.println("ESP-NOW: Beide Empfänger registriert.");
}

// Send command via ESP-NOW with retry mechanism
bool sendESPNowCommand(int16_t speed, int16_t steer, uint8_t cmd) {
    lastCommandTime = millis();
    
    ESPNowCommand data;
    // Speicher leeren, um Garbage-Data im Padding zu verhindern
    memset(&data, 0, sizeof(ESPNowCommand));

    data.start = 0xABCD; // Muss exakt mit Receiver START_FRAME übereinstimmen
    data.steer = steer;
    data.speed = speed;
    data.commandCode = cmd;

    // FIX: Strikte Checksummen-Berechnung mit Casts auf (uint16_t)
    // Dies verhindert Fehler bei negativen Werten oder großen PWM Werten (wie 25600)
    data.checksum = data.start ^ (uint16_t)data.steer ^ (uint16_t)data.speed ^ (uint16_t)data.commandCode;
    
    // Status zurücksetzen
    deliverySuccess = false;
    deliveryFailed = false;
    
    esp_err_t result = esp_now_send(activeReceiver, (uint8_t *)&data, sizeof(data));
    
    // Kurzes Warten auf Bestätigung (non-blocking für Joystick-Fluidität)
    unsigned long startWait = millis();
    while (!deliverySuccess && !deliveryFailed && (millis() - startWait < 5)) {
        delay(0); // Yield
    }

    // Statistik für Kanalwechsel (wie in deinem Code)
    static int failCounter = 0;
    if (!deliverySuccess) {
        failCounter++;
    } else {
        failCounter = 0;
    }

    /*
    if (failCounter >= 200) { 
        activeReceiver = (activeReceiver == receiver1) ? receiver2 : receiver1;
        // Serial.println("Switching Receiver Target due to packet loss."); 
        failCounter = 0; 
    }
    */

    return (result == ESP_OK);
}

/* OnConnect Callback: Handles controller connection */
void onConnect() {
    isControllerConnected = true;  // Set the controller connection status to true
    Serial.println("PS3 Controller Connected!");
    digitalWrite(LED_PIN, HIGH);   // Keep LED on steady
}

/* Notify Callback: Handles PS3 controller events */
void notify() {

    // ===== Safety Check (L1 as Deadman Switch) =====
    if (L1_IS_DEADMAN_SWITCH && !Ps3.data.button.l1) {
        Serial.println("L1 (Deadman) not pressed: Blocking commands.");
        speed = 0;
        steer = 0;
        cruiseControlSpeed = false;
        cruiseControlSteer = false;
        sendESPNowCommand(0, 20, 8); // Send emergency stop (Skill 8, param 20)
        return; 
    }

    // ===== L1 as Emergency Stop button =====
    if (!L1_IS_DEADMAN_SWITCH && Ps3.event.button_down.l1) {
    // Parameter: Speed=0, Steer=20, CommandCode=8
    // Der Receiver interpretiert Steer als param1 -> param1 ist 20 -> E-Stop triggered.
    if(sendESPNowCommand(0, 20, 8)) {
        Serial.println("!!! L1 NOT-AUS gesendet (Skill 8) !!!");
    } else {
        Serial.println("!!! FEHLER: Not-Aus konnte nicht gesendet werden !!!");
    }
    return;
    }

    // ===== Button Handlers (Immediate Return) =====
    if (Ps3.event.button_down.cross) {
        steeringTrim = 0;       // Reset local steering trim
        cruiseControlSpeed = false; // Deactivate speed cruise control
        cruiseControlSteer = false; // Deactivate steer cruise control
        sendESPNowCommand(0, 0, 1);
        Serial.println("Cross button pressed: All trims & cruise controls reset. System Reset (Skill 1) sent.");
        return;
    }

    // ===== Two-Step Turn Logic: Prepare with Square button =====
    if (Ps3.event.button_down.square) {
        preparedTurnType = 90;
        Serial.println("90-degree turn PREPARED. Use D-Pad Left/Right to execute.");
        return;
    }

    // ===== Two-Step Turn Logic: Prepare with Circle button =====
    if (Ps3.event.button_down.circle) {
        preparedTurnType = 180;
        Serial.println("180-degree turn PREPARED. Use D-Pad Left/Right to execute.");
        return;
    }

    if (Ps3.event.button_down.triangle) {
        steer = 0;
        sendESPNowCommand(speed, steer, 3);
        Serial.println("Triangle button pressed: Maintain straight path.");
        return;
    }

    // ===== Start button sends Receiver Reset (9), then self-resets =====
    if (Ps3.event.button_down.start) {
        Serial.println("Start button pressed. Sending Reset command (9) to receiver...");
        sendESPNowCommand(0, 0, 9); 
        Serial.println("Receiver reset command sent. Remote will restart in 1 second.");
        delay(1000);
        ESP.restart(); 
        return; 
    }
    
    // ===== Select Button: Blade Stand-by/ON (Code 6, Param 1) =====
    if (Ps3.event.button_down.select) {
        bladeActive = true; 
        sendESPNowCommand(0, 1, 6); 
        Serial.println("Select: Messer Stand-by/ON (Code 6, Param 1)");
        return;
    }

    // ===== L2 Button: Messer AUS (Code 6, Param -1) =====
    if (Ps3.event.button_down.l2) {
        bladeActive = false;
        sendESPNowCommand(0, -1, 6); 
        Serial.println("L2: Messer AUS (Code 6, Param -1)");
        return;
    }

    // ===== R1 Button: Messer Speed 1 (Code 6, PWM 25500) =====
    if (Ps3.event.button_down.r1) {
        // Nutzt den "Neuner-Filter" des Receivers: 25500 + 65536 - 90000 = 1036 PWM (Beispielwert)
        // Sende hier den spezifischen Key für Speed 1
        sendESPNowCommand(0, 25600, 6); 
        Serial.println("R1: Messer Speed 1 (Key 25600)");
        return;
    }

    // ===== R2 Button: Messer Speed 2 (Code 6, PWM 26500) =====
    if (Ps3.event.button_down.r2) {
        sendESPNowCommand(0, 26500, 6); 
        Serial.println("R2: Messer Speed 2 (Key 26500)");
        return;
    }

    // ===== D-Pad Handlers =====
    if (Ps3.event.button_down.up) {
        speedMode = min(speedMode + 1, 5);
        Serial.print("D-Pad Up pressed: Speed Mode changed to ");
        Serial.println(speedMode);
        return;
    }
    if (Ps3.event.button_down.down) {
        speedMode = max(speedMode - 1, 1);
        Serial.print("D-Pad Down pressed: Speed Mode changed to ");
        Serial.println(speedMode);
        return;
    }
    
    // ===== Two-Step Turn Logic: Execute with D-Pad or Trim =====
    if (Ps3.event.button_down.left) {
        if (preparedTurnType > 0) {
            uint8_t cmdCode = (preparedTurnType == 90) ? 2 : 4;
            sendESPNowCommand(speed, -1, cmdCode); 
            Serial.printf("%d-degree turn LEFT EXECUTED.\n", preparedTurnType);
            preparedTurnType = 0; 
        } else {
            steeringTrim = max(steeringTrim - TRIM_STEP, -MAX_STEER);
            Serial.print("D-Pad Left pressed: Steering trim decreased to ");
            Serial.println(steeringTrim);
        }
        return;
    }
    if (Ps3.event.button_down.right) {
        if (preparedTurnType > 0) {
            uint8_t cmdCode = (preparedTurnType == 90) ? 2 : 4;
            sendESPNowCommand(speed, 1, cmdCode);
            Serial.printf("%d-degree turn RIGHT EXECUTED.\n", preparedTurnType);
            preparedTurnType = 0; 
        } else {
            steeringTrim = min(steeringTrim + TRIM_STEP, MAX_STEER);
            Serial.print("D-Pad Right pressed: Steering trim increased to ");
            Serial.println(steeringTrim);
        }
        return;
    }

    // ===== Stick Buttons =====
    if (Ps3.event.button_down.l3) {
        cruiseControlSpeed = !cruiseControlSpeed;
        Serial.println(cruiseControlSpeed 
            ? "Cruise control for speed ACTIVATED" 
            : "Cruise control for speed DEACTIVATED");
        return;
    }
    if (Ps3.event.button_down.r3) {
        cruiseControlSteer = !cruiseControlSteer;
        Serial.println(cruiseControlSteer 
            ? "Cruise control for steering ACTIVATED" 
            : "Cruise control for steering DEACTIVATED");
        return;
    }


    // ===== Joystick Processing =====
      
      int modes[] = {0, 100, 150, 200, 400, 800}; 
      int maxModeVal = modes[speedMode]; 

      // 1. Rohwerte holen
      float rawLY = (float)Ps3.data.analog.stick.ly;
      float rawRX = (float)Ps3.data.analog.stick.rx;

      // 2. Normalisieren auf -1.0 bis +1.0
      // PS3 Werte gehen von -128 bis 127. 
      float normY = -(rawLY / 127.0f);
      float normX = (rawRX / 127.0f);

      // 3. Deadzone (10%)
      if (abs(normY) < 0.1f) normY = 0;
      if (abs(normX) < 0.1f) normX = 0;

      // 4. Square Mapping (Sorgt für vollen Ausschlag in den Diagonalen)
      // Wir skalieren die Achsen so, dass sie sich wie ein Quadrat verhalten, nicht wie ein Kreis.
      if (!cruiseControlSpeed) {
          int16_t targetSpeed = (int16_t)(normY * maxModeVal);
          speed = (drivingDirection == 1) ? targetSpeed : -targetSpeed;
      }
      
      if (!cruiseControlSteer) {
          int16_t targetSteer = (int16_t)(normX * maxModeVal);
          steer = constrain(targetSteer + steeringTrim, -MAX_STEER, MAX_STEER);
      }

    // ===== Consolidated & Rate-Limited Sending Logic =====
    
    bool joystickActive = shouldSendJoystickCommand(speed, steer);
    bool cruiseActive = (cruiseControlSpeed && speed != 0) || (cruiseControlSteer && steer != 0);
    bool isCurrentlyActive = joystickActive || cruiseActive;

     static bool wasPreviouslyActive = false;

    if (wasPreviouslyActive && !isCurrentlyActive) {
        // SICHERHEIT: Stop-Befehl 5x als "Salve" senden, falls ein Paket verloren geht
        for (int i = 0; i < 5; i++) {
            sendESPNowCommand(0, 0, 0);
            delay(10); // Kurze Pause zwischen den Paketen (Flood protection)
        }
        Serial.println("Joystick/Cruise stopped - Sent zero command (x5 Burst)");
    }
    else if (isCurrentlyActive && (millis() - lastSendTime >= 50)) {
        sendESPNowCommand(speed, steer, 0); 
        Serial.printf("Sending -> Speed: %d | Steer: %d\n", speed, steer);
        lastSendTime = millis();
    } 

    wasPreviouslyActive = isCurrentlyActive;
}

/* Setup Function */
void setup() {
    Serial.begin(115200);
    while(!Serial); 
    pinMode(LED_PIN, OUTPUT);
    
    WiFi.mode(WIFI_STA);
    
    // Force WiFi channel for ESP-NOW stability
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
    
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.print("WiFi Channel: ");
    Serial.println(WiFi.channel());

    initESPNow();

    Ps3.attach(notify);
    Ps3.attachOnConnect(onConnect);
    Ps3.begin("20:00:00:00:01:20"); 
    Serial.println("Waiting for PS3 controller...");
}

/* Main Loop */
void loop() {
    // 1. Serial Monitor Reset Logic
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim(); // Leerzeichen entfernen
        if (input.equalsIgnoreCase("reset")) {
            Serial.println("Reset-Befehl via Serial empfangen. Starte neu...");
            delay(500);
            ESP.restart();
        }
    }

    // 2. Verbindungs-Management
    // Fall A: Verbindung verloren?
    if (isControllerConnected && !Ps3.isConnected()) {
        isControllerConnected = false;
        Serial.println("PS3 Controller Verbindung verloren/getrennt.");
    }

    // Fall B: Warten auf Verbindung (Blinken)
    if (!isControllerConnected) {
        digitalWrite(LED_PIN, millis() % 1000 < 500 ? HIGH : LOW);
        
        if (Ps3.isConnected()) {
            isControllerConnected = true;
            digitalWrite(LED_PIN, HIGH);
            Serial.println("PS3 Controller verbunden.");
        }
    }

    // 3. Heartbeat (nur wenn verbunden)
    unsigned long currentMillis = millis();
    if (isControllerConnected && (currentMillis - lastCommandTime >= 10000)) {
        sendESPNowCommand(0, 0, 255);
        Serial.println("Heartbeat sent (10s inactivity)");
    }
    
    delay(20);
}

*/