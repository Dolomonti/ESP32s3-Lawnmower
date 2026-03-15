/*
 * NetworkManager.h
 * 
 * Phase 4.5: Netzwerk Management (WiFi & WebServer)
 * 
 * HINWEIS: Dies ist eine vereinfachte Version. Komplexe WebSocket-Handler
 * und HTTP-Routen mit Lambdas bleiben in main.cpp, da sie auf viele
 * globale Variablen und Funktionen zugreifen.
 */

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>

// ====================================================================================
// ===== EXTERNE VARIABLEN =============================================================
// ====================================================================================
extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern DNSServer dnsServer;
extern char stored_ssid[32];
extern char stored_password[32];
extern bool isWiFiConnected;

// ====================================================================================
// ===== FUNKTIONEN ====================================================================
// ====================================================================================

// Initialisiert WiFi im AP-Modus (für Erstkonfiguration)
void setupWiFiAP();

// Verbindet mit Heimnetzwerk (STA-Modus)
// Gibt true zurück bei erfolgreicher Verbindung
bool connectToWiFiSTA(const char* ssid, const char* password, int maxRetries = 10);

// Startet DNS-Server (für Captive Portal im AP-Modus)
void startDNSServer();

// Verarbeitet DNS-Anfragen (muss in loop() aufgerufen werden)
void handleDNS();

// Trennt WiFi-Verbindung
void disconnectWiFi();

// Gibt aktuelle IP-Adresse zurück (oder "0.0.0.0" wenn nicht verbunden)
String getLocalIPString();

// Prüft ob mit Heimnetzwerk verbunden
bool isConnectedToHomeNetwork();

#endif // NETWORKMANAGER_H
