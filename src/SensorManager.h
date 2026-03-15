/*
 * SensorManager.h
 * 
 * Phase 4.4: Sensor Management (MPU6050 Initialisierung & Spannungsmessung)
 * 
 * HINWEIS: mpuReadTask bleibt in main.cpp, da sie direkt auf viele 
 * globale Variablen und Skill-Funktionen zugreift.
 */

#ifndef SENSORMANAGER_H
#define SENSORMANAGER_H

#include <Arduino.h>

// ====================================================================================
// ===== FUNKTIONEN ====================================================================
// ====================================================================================

// Liest aktuelle Blade-Batterie-Spannung (in Millivolt)
// Nutzt den konfigurierten bladeBatteryFactor aus Settings
int32_t readBladeBatteryVoltage();

// Glättet einen Spannungswert mit Exponentialfilter
// alpha: Glättungsfaktor (0.0-1.0), kleiner = glatter
float smoothVoltage(float newValue, float smoothedValue, float alpha);

#endif // SENSORMANAGER_H
