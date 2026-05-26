// ╔══════════════════════════════════════════════════════╗
// ║         AI CAREON — actuators.h                      ║
// ║         Physical Response System                     ║
// ╚══════════════════════════════════════════════════════╝
#pragma once

#include <Arduino.h>
#include "config.h"

// ── Initialize All Actuator Pins ────────────────────────
void initActuators() {
  pinMode(PIN_RELAY1,      OUTPUT);
  pinMode(PIN_RELAY2,      OUTPUT);
  pinMode(PIN_RELAY3,      OUTPUT);
  pinMode(PIN_RELAY4,      OUTPUT);
  pinMode(PIN_BUZZER,      OUTPUT);
  pinMode(PIN_LED_GREEN,   OUTPUT);
  pinMode(PIN_LED_YELLOW,  OUTPUT);
  pinMode(PIN_LED_RED,     OUTPUT);
  pinMode(PIN_LED_PUMP,    OUTPUT);
  pinMode(PIN_LED_VIB,     OUTPUT);
  pinMode(PIN_LED_ACT,     OUTPUT);

  // Relay modules are ACTIVE LOW — HIGH = relay OFF (safe default)
  digitalWrite(PIN_RELAY1,     HIGH);
  digitalWrite(PIN_RELAY2,     HIGH);
  digitalWrite(PIN_RELAY3,     HIGH);
  digitalWrite(PIN_RELAY4,     HIGH);
  digitalWrite(PIN_BUZZER,     LOW);
  digitalWrite(PIN_LED_GREEN,  LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_RED,    LOW);
  digitalWrite(PIN_LED_PUMP,   LOW);
  digitalWrite(PIN_LED_VIB,    LOW);
  digitalWrite(PIN_LED_ACT,    LOW);

  Serial.println("[Actuator] All actuators initialized");
}

// ══════════════════════════════════════════════════════
// AIR PUMP — Inflate for 30s then deflate
// ══════════════════════════════════════════════════════
unsigned long pump_start   = 0;
bool          pump_running = false;

void triggerAirPump() {
  if (!pump_running) {
    digitalWrite(PIN_RELAY1, LOW);   // Turn pump ON
    digitalWrite(PIN_LED_PUMP, HIGH);
    pump_start   = millis();
    pump_running = true;
    Serial.println("[Actuator] Air pump ON — inflating bladders");
  }
}

// Call this every loop() — non-blocking pump cycle
void updateAirPump() {
  if (pump_running && (millis() - pump_start > 30000)) {
    digitalWrite(PIN_RELAY1,  HIGH);  // Stop pump
    digitalWrite(PIN_RELAY2,  LOW);   // Open release valve
    delay(5000);                       // Release for 5s
    digitalWrite(PIN_RELAY2,  HIGH);  // Close valve
    digitalWrite(PIN_LED_PUMP, LOW);
    pump_running = false;
    Serial.println("[Actuator] Air pump cycle complete");
  }
}

// ══════════════════════════════════════════════════════
// VIBRATION — Gentle 3-second burst
// ══════════════════════════════════════════════════════
void triggerVibration() {
  digitalWrite(PIN_LED_VIB, HIGH);
  ledcWrite(0, 180);     // PWM channel 0, ~70% duty — gentle
  delay(3000);
  ledcWrite(0, 0);
  digitalWrite(PIN_LED_VIB, LOW);
  Serial.println("[Actuator] Vibration cycle done");
}

// ══════════════════════════════════════════════════════
// LINEAR ACTUATOR — Raise / Lower bed section
// ══════════════════════════════════════════════════════
bool bed_raised = false;

void raiseBed() {
  if (!bed_raised) {
    digitalWrite(PIN_RELAY3,   LOW);   // Direction A
    digitalWrite(PIN_RELAY4,   HIGH);
    digitalWrite(PIN_LED_ACT,  HIGH);
    delay(3000);                        // Run 3s = ~15mm movement
    digitalWrite(PIN_RELAY3,   HIGH);  // Stop
    bed_raised = true;
    Serial.println("[Actuator] Bed raised — weight redistributed");
  }
}

void lowerBed() {
  if (bed_raised) {
    digitalWrite(PIN_RELAY4,  LOW);    // Direction B
    digitalWrite(PIN_RELAY3,  HIGH);
    delay(3000);
    digitalWrite(PIN_RELAY4,  HIGH);   // Stop
    digitalWrite(PIN_LED_ACT, LOW);
    bed_raised = false;
    Serial.println("[Actuator] Bed lowered");
  }
}

// ══════════════════════════════════════════════════════
// ALERT LEDs + BUZZER
// ══════════════════════════════════════════════════════
void setAlert(int risk_level) {
  // risk_level: 0=safe  1=caution  2=danger  3=critical
  digitalWrite(PIN_LED_GREEN,  risk_level == 0 ? HIGH : LOW);
  digitalWrite(PIN_LED_YELLOW, risk_level == 1 ? HIGH : LOW);
  digitalWrite(PIN_LED_RED,    risk_level >= 2 ? HIGH : LOW);
  digitalWrite(PIN_BUZZER,     risk_level >= 3 ? HIGH : LOW);
}

// ══════════════════════════════════════════════════════
// MASTER RESPONSE — Triggered by risk score (0–100)
// ══════════════════════════════════════════════════════
void executeResponse(int score) {
  if (score >= 80) {
    // CRITICAL — full intervention
    raiseBed();
    triggerAirPump();
    setAlert(3);
    Serial.println("[Response] CRITICAL — Bed raised + Air pump + Alert");

  } else if (score >= 60) {
    // DANGER — air pump + alert
    triggerAirPump();
    setAlert(2);
    Serial.println("[Response] DANGER — Air pump activated + Alert");

  } else if (score >= 40) {
    // CAUTION — vibration + yellow LED
    triggerVibration();
    setAlert(1);
    Serial.println("[Response] CAUTION — Vibration triggered");

  } else {
    // SAFE — lower bed if raised, green LED
    if (bed_raised) lowerBed();
    setAlert(0);
  }
}
