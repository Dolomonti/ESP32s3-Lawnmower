# ESP32-S3 Lawnmower Control System

**GitHub Repository:** https://github.com/Dolomonti/ESP32s3-Lawnmower

## Projektübersicht

ESP32-S3 basiertes Steuerungssystem für einen autonomen/ferngesteuerten Rasenmäher.

### Hardware
- **MCU:** ESP32-S3-DevKitC-1
- **Antrieb:** Hoverboard Hub-Motoren (2x 350W) über UART
- **Mähmotor:** Brushless Motor (SURPASS-HOBBY C3548-V2) mit 120A ESC
- **IMU:** MPU6050 für Lageerkennung und Kippschutz
- **Kommunikation:** WiFi Web Interface + ESP-NOW Fernsteuerung

### Features
- Web-basierte Steuerung über WiFi AP
- Echtzeit-Sensor-Daten (Spannung, Temperatur, Geschwindigkeit)
- Kippschutz mit automatischem Not-Stop
- 20+ Skills für autonome Missionen
- ESP-NOW Fernsteuerung

### Build
```bash
pio run -e esp32s3
pio run --target upload -e esp32s3
```

## Autor
Markus Montagner
