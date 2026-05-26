// ╔══════════════════════════════════════════════════════╗
// ║         AI CAREON — sensors.h                        ║
// ║         All Sensor Reading Functions                 ║
// ╚══════════════════════════════════════════════════════╝
#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <MPU6050.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "HX711.h"
#include "config.h"

// ── Sensor Objects ──────────────────────────────────────
DHT      dht(PIN_DHT, DHT22);
MPU6050  mpu;
MAX30105 pulse;
HX711    loadCell;

// ── Sensor Data Structure ───────────────────────────────
struct SensorData {
  int   fsr_raw;         // 0–4095  (12-bit ADC on ESP32)
  float temperature;     // Celsius
  float humidity;        // Percent
  float accel_x;         // g
  float accel_y;         // g
  float accel_z;         // g
  float weight_kg;
  int   heart_rate;      // BPM
  float spo2;            // Percent
  bool  moisture_wet;    // true = wet surface detected
  unsigned long timestamp;
};

// ── Initialize All Sensors ──────────────────────────────
void initSensors() {
  Wire.begin(PIN_SDA, PIN_SCL);

  // DHT22
  dht.begin();

  // MPU6050
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("[Sensor] MPU6050 connected OK");
  } else {
    Serial.println("[Sensor] MPU6050 FAILED — check wiring");
  }

  // MAX30102
  if (!pulse.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("[Sensor] MAX30102 not found — check wiring");
  } else {
    Serial.println("[Sensor] MAX30102 connected OK");
    pulse.setup();
    pulse.setPulseAmplitudeRed(0x0A);
  }

  // HX711 Load Cell
  loadCell.begin(33, 32);          // DOUT=33, SCK=32 — adjust if needed
  loadCell.set_scale(-7050.0);     // Your calibration factor
  loadCell.tare();
  Serial.println("[Sensor] HX711 load cell ready");

  // Moisture sensor pin
  pinMode(PIN_MOISTURE, INPUT);

  Serial.println("[Sensor] All sensors initialized");
}

// ── Read All Sensors ────────────────────────────────────
SensorData readAllSensors() {
  SensorData d;
  d.timestamp = millis();

  // FSR Pressure (ESP32 ADC: 0–4095)
  d.fsr_raw = analogRead(PIN_FSR);

  // DHT22 Temperature & Humidity
  d.temperature = dht.readTemperature();
  d.humidity    = dht.readHumidity();
  if (isnan(d.temperature)) d.temperature = 0.0;
  if (isnan(d.humidity))    d.humidity    = 0.0;

  // MPU6050 Motion & Posture
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  d.accel_x = ax / 16384.0;   // Convert raw to g
  d.accel_y = ay / 16384.0;
  d.accel_z = az / 16384.0;

  // HX711 Load Cell Weight
  if (loadCell.is_ready()) {
    d.weight_kg = loadCell.get_units(5);
  }

  // MAX30102 Heart Rate & SpO2
  long ir = pulse.getIR();
  if (ir > 50000) {             // Finger detected
    d.heart_rate = (int)checkForBeat(ir);
    d.spo2       = 98.0;        // Use full SpO2 algorithm for accuracy
  } else {
    d.heart_rate = 0;
    d.spo2       = 0.0;
  }

  // Moisture Sensor
  int moist_raw  = analogRead(PIN_MOISTURE);
  d.moisture_wet = (moist_raw > MOISTURE_WET);

  return d;
}
