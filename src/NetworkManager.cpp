/*
 * NetworkManager.cpp
 * 
 * Phase 4.5: Netzwerk Management Implementierung
 */

#include "NetworkManager.h"
#include "Config.h"
#include "Globals.h"

// Debug-Ausgaben (DEBUG_LOG ist in main.cpp definiert)
#define NM_LOG(msg) Serial.println(msg)
#define NM_PRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)

// ====================================================================================
// ===== IMPLEMENTIERUNGEN =============================================================
// ====================================================================================

void setupWiFiAP() {
    WiFi.mode(WIFI_MODE_APSTA);
    
    const char* ssid = "Lawnmower_Control";
    const char* password = "123456789";
    IPAddress local_IP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    
    // Statische IP-Konfiguration für schnelleren Verbindungsaufbau
    if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
        NM_LOG("[WiFiAP] AP Config Failed");
    }
    
    // Access Point auf Kanal 6 starten (Wichtig für ESP-NOW)
    if (WiFi.softAP(ssid, password, 6, 0, 4)) {
        // DNS Server starten (Captive Portal)
        dnsServer.start(53, "*", local_IP);
        
        NM_LOG("[WiFiAP] Access Point started successfully");
        NM_PRINTF("[WiFiAP] SSID: %s | Password: %s\n", ssid, password);
        NM_PRINTF("[WiFiAP] AP IP Address: %s\n", WiFi.softAPIP().toString().c_str());
    } else {
        NM_LOG("[WiFiAP] Failed to start Access Point!");
    }
}

bool connectToWiFiSTA(const char* ssid, const char* password, int maxRetries) {
    if (strlen(ssid) == 0) {
        return false;
    }
    
    NM_PRINTF("Connecting to WiFi: %s\n", ssid);
    
    // Phase 3: DHCP statt statischer IP
    WiFi.begin(ssid, password);
    
    int retryCount = 0;
    const int retryDelay = 1000; // 1 second
    
    while (retryCount < maxRetries) {
        if (WiFi.status() == WL_CONNECTED) {
            NM_LOG("Connected to WiFi successfully!");
            NM_PRINTF("IP address: %s\n", WiFi.localIP().toString().c_str());
            return true;
        }
        
        retryCount++;
        NM_PRINTF("WiFi connection attempt %d/%d failed, retrying...\n", retryCount, maxRetries);
        delay(retryDelay);
    }
    
    NM_LOG("Failed to connect to WiFi after all retries");
    return false;
}

void startDNSServer() {
    // DNS-Server starten (optional für Captive Portal)
    // Wird in main.cpp initialisiert falls benötigt
}

void handleDNS() {
    // DNS-Server verarbeiten
    // Wird in main.cpp aufgerufen falls DNS aktiviert
}

void disconnectWiFi() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    NM_LOG("WiFi disconnected");
}

String getLocalIPString() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

bool isConnectedToHomeNetwork() {
    return (WiFi.status() == WL_CONNECTED);
}
