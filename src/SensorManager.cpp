/*
 * SensorManager.cpp
 * 
 * Phase 4.4: Sensor Management Implementierung
 */

#include "SensorManager.h"
#include "Config.h"
#include "Globals.h"

int32_t readBladeBatteryVoltage() {
    // Spannung am Pin lesen (Millivolt)
    uint32_t raw_pin_mv = analogReadMilliVolts(BLADE_BATTERY_PIN);
    
    // Mit Kalibrierfaktor multiplizieren
    float calculated_mv = (float)raw_pin_mv * currentSettings.bladeBatteryFactor;
    
    return (int32_t)calculated_mv;
}

float smoothVoltage(float newValue, float smoothedValue, float alpha) {
    if (smoothedValue < 5000) {
        // Erster Wert oder Reset - sofort übernehmen
        return newValue;
    }
    // Exponentialfilter: neuer Wert = (alt * (1-alpha)) + (neu * alpha)
    return (smoothedValue * (1.0f - alpha)) + (newValue * alpha);
}
