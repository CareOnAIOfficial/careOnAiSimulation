// ╔══════════════════════════════════════════════════════╗
// ║         AI CAREON — algorithms.h                     ║
// ║         Intelligence Layer — All Algorithms          ║
// ╚══════════════════════════════════════════════════════╝
#pragma once

#include <Arduino.h>
#include "sensors.h"
#include "config.h"

// ══════════════════════════════════════════════════════
// 1. MOVING AVERAGE FILTER
//    Smooths sensor readings to eliminate noise spikes
// ══════════════════════════════════════════════════════
#define MA_WINDOW 10

int   fsr_history[MA_WINDOW]  = {0};
float temp_history[MA_WINDOW] = {0};
int   ma_index = 0;

int movingAvgFSR(int new_val) {
  fsr_history[ma_index] = new_val;
  ma_index = (ma_index + 1) % MA_WINDOW;
  long sum = 0;
  for (int i = 0; i < MA_WINDOW; i++) sum += fsr_history[i];
  return (int)(sum / MA_WINDOW);
}

float movingAvgTemp(float new_val) {
  temp_history[ma_index % MA_WINDOW] = new_val;
  float sum = 0;
  for (int i = 0; i < MA_WINDOW; i++) sum += temp_history[i];
  return sum / MA_WINDOW;
}

// ══════════════════════════════════════════════════════
// 2. TIME ANALYSIS
//    Tracks how long dangerous pressure has been applied
//    Duration is the real danger — not just magnitude
// ══════════════════════════════════════════════════════
unsigned long pressure_start_time = 0;
bool          pressure_active      = false;
unsigned long pressure_duration    = 0;   // seconds

void updatePressureTime(bool is_dangerous) {
  if (is_dangerous) {
    if (!pressure_active) {
      pressure_start_time = millis();
      pressure_active     = true;
    }
    pressure_duration = (millis() - pressure_start_time) / 1000;
  } else {
    pressure_active   = false;
    pressure_duration = 0;
  }
}

// ══════════════════════════════════════════════════════
// 3. IMMOBILITY DETECTION
//    Returns true if patient has not moved long enough
// ══════════════════════════════════════════════════════
float         last_ax = 0, last_ay = 0, last_az = 0;
unsigned long last_movement_time = 0;

#define MOVEMENT_THRESHOLD  0.05    // g — movement sensitivity

bool checkImmobility(float ax, float ay, float az) {
  float delta = abs(ax - last_ax) + abs(ay - last_ay) + abs(az - last_az);

  if (delta > MOVEMENT_THRESHOLD) {
    last_movement_time = millis();
    last_ax = ax;
    last_ay = ay;
    last_az = az;
    return false;   // Patient moved — safe
  }

  unsigned long still_seconds = (millis() - last_movement_time) / 1000;
  return (still_seconds > NO_MOVE_LIMIT);
}

// ══════════════════════════════════════════════════════
// 4. RULE-BASED ENGINE
//    Immediate threshold violations
//    Returns risk level: 0=safe 1=caution 2=danger 3=critical
// ══════════════════════════════════════════════════════
int ruleBasedRisk(SensorData& d, int avg_fsr) {
  int risk = 0;

  // Pressure rules
  if (avg_fsr > PRESSURE_THRESHOLD) risk = max(risk, 1);
  if (avg_fsr > PRESSURE_DANGER)    risk = max(risk, 2);

  // Duration rules
  if (pressure_duration > PRESSURE_TIME_CAUTION) risk = max(risk, 2);
  if (pressure_duration > PRESSURE_TIME_LIMIT)   risk = 3;

  // Temperature rules
  if (d.temperature > TEMP_HIGH) risk = max(risk, 1);
  if (d.temperature > 39.0)      risk = max(risk, 2);

  // Humidity rule
  if (d.humidity > HUMIDITY_HIGH) risk = max(risk, 1);

  // SpO2 rules
  if (d.spo2 > 0 && d.spo2 < SPO2_LOW) risk = max(risk, 2);
  if (d.spo2 > 0 && d.spo2 < 90.0)     risk = 3;

  // Moisture rule
  if (d.moisture_wet) risk = max(risk, 1);

  return risk;
}

// ══════════════════════════════════════════════════════
// 5. DECISION TREE — COMPOSITE RISK SCORE (0–100)
//    Combines all factors into one number
//
//    Score breakdown:
//    Pressure   → 0–40 points
//    Duration   → 0–25 points
//    Immobility → 0–20 points
//    Temperature→ 0–10 points
//    SpO2       → 0–10 points  (only if sensor worn)
//    Moisture   → 0–5  points
//    ─────────────────────────
//    Total max  → 100 points
// ══════════════════════════════════════════════════════
int decisionTree(SensorData& d, int avg_fsr, bool immobile) {
  int score = 0;

  // Pressure contribution (0–40)
  if      (avg_fsr > 3500) score += 40;
  else if (avg_fsr > 2800) score += 25;
  else if (avg_fsr > 1800) score += 10;

  // Duration contribution (0–25)
  if      (pressure_duration > PRESSURE_TIME_LIMIT)   score += 25;
  else if (pressure_duration > PRESSURE_TIME_DANGER)  score += 15;
  else if (pressure_duration > PRESSURE_TIME_CAUTION) score += 8;

  // Immobility contribution (0–20)
  if (immobile) score += 20;

  // Temperature contribution (0–10)
  if      (d.temperature > 39.0)      score += 10;
  else if (d.temperature > TEMP_HIGH) score += 5;

  // SpO2 contribution (0–10)
  if      (d.spo2 > 0 && d.spo2 < 90.0)      score += 10;
  else if (d.spo2 > 0 && d.spo2 < SPO2_LOW)  score += 5;

  // Moisture contribution (0–5)
  if (d.moisture_wet) score += 5;

  return min(score, 100);
}

// ══════════════════════════════════════════════════════
// HELPER — Risk label string
// ══════════════════════════════════════════════════════
String riskLabel(int score) {
  if      (score >= 80) return "CRITICAL";
  else if (score >= 60) return "DANGER";
  else if (score >= 40) return "CAUTION";
  else                  return "SAFE";
}
