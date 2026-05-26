// ╔══════════════════════════════════════════════════════╗
// ║         AI CAREON — main.cpp                         ║
// ║         Main Firmware — ESP32                        ║
// ║                                                      ║
// ║  Ties all modules together:                          ║
// ║  Sensors → Algorithms → Actuators → Cloud           ║
// ╚══════════════════════════════════════════════════════╝

#include <Arduino.h>
#include "config.h"
#include "sensors.h"
#include "algorithms.h"
#include "actuators.h"
#include "cloud.h"

// ── Timing Constants ────────────────────────────────────
#define SAMPLE_INTERVAL   5000    // Read sensors every 5 seconds
#define UPLOAD_INTERVAL   30000   // Upload to Firebase every 30 seconds
#define ALERT_COOLDOWN    300000  // 5 minutes between repeated alerts

// ── Timing Trackers ─────────────────────────────────────
unsigned long last_sample = 0;
unsigned long last_upload = 0;
unsigned long last_alert  = 0;

// ── Setup ───────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("╔═══════════════════════════════════════╗");
  Serial.println("║        AI CAREON v1.0 — ESP32         ║");
  Serial.println("║   Bed Sore Prevention System          ║");
  Serial.println("╚═══════════════════════════════════════╝");
  Serial.println();

  // Initialize all systems
  initActuators();
  initSensors();
  initWiFi();
  initFirebase();
  initBlynk();

  // PWM setup for vibration motor (ESP32 uses ledcSetup)
  ledcSetup(0, 5000, 8);           // Channel 0, 5kHz, 8-bit
  ledcAttachPin(PIN_LED_VIB, 0);   // Attach vibration pin

  Serial.println();
  Serial.println("[System] All systems ready — monitoring started");
  Serial.println("─────────────────────────────────────────────");
}

// ── Main Loop ───────────────────────────────────────────
void loop() {
  // Keep Blynk connection alive
  Blynk.run();

  // Handle air pump timing (non-blocking)
  updateAirPump();

  unsigned long now = millis();

  // ── Sample sensors every 5 seconds ──────────────────
  if (now - last_sample >= SAMPLE_INTERVAL) {
    last_sample = now;

    // ── STEP 1: Read all sensors ─────────────────────
    SensorData data = readAllSensors();

    // ── STEP 2: Apply moving average filter ──────────
    int avg_fsr = movingAvgFSR(data.fsr_raw);

    // ── STEP 3: Check immobility ──────────────────────
    bool immobile = checkImmobility(
      data.accel_x, data.accel_y, data.accel_z
    );

    // ── STEP 4: Update pressure time tracking ─────────
    bool pressure_dangerous = (avg_fsr > PRESSURE_THRESHOLD);
    updatePressureTime(pressure_dangerous);

    // ── STEP 5: Run rule-based engine ─────────────────
    int rule_risk = ruleBasedRisk(data, avg_fsr);

    // ── STEP 6: Run decision tree → composite score ───
    int risk_score = decisionTree(data, avg_fsr, immobile);

    // ── STEP 7: Execute physical response ─────────────
    executeResponse(risk_score);

    // ── STEP 8: Print to Serial Monitor ───────────────
    Serial.println();
    Serial.print("[Reading]  FSR:");
    Serial.print(avg_fsr);
    Serial.print("  Temp:");
    Serial.print(data.temperature, 1);
    Serial.print("C  Hum:");
    Serial.print(data.humidity, 1);
    Serial.print("%  HR:");
    Serial.print(data.heart_rate);
    Serial.print("  SpO2:");
    Serial.print(data.spo2, 1);
    Serial.print("%  Wet:");
    Serial.print(data.moisture_wet ? "YES" : "NO");
    Serial.print("  Still:");
    Serial.print(pressure_duration);
    Serial.println("s");

    Serial.print("[Score]    ");
    Serial.print(risk_score);
    Serial.print("/100  →  ");
    Serial.println(riskLabel(risk_score));

    // ── STEP 9: Send to Blynk ─────────────────────────
    sendToBlynk(data, risk_score, pressure_duration);

    // ── STEP 10: Send alert if needed (with cooldown) ──
    if (rule_risk >= 2 && (now - last_alert > ALERT_COOLDOWN)) {
      sendAlert(rule_risk, risk_score);
      last_alert = now;
    }

    // ── STEP 11: Upload to Firebase every 30 seconds ───
    if (now - last_upload >= UPLOAD_INTERVAL) {
      uploadToFirebase(data, risk_score, pressure_duration);
      last_upload = now;
    }
  }
}
