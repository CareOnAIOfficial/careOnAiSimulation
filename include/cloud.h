// ╔══════════════════════════════════════════════════════╗
// ║         AI CAREON — cloud.h                          ║
// ║         Firebase + Blynk Cloud Integration           ║
// ╚══════════════════════════════════════════════════════╝
#define FIREBASE_HOST   "ai-careon-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH   "x3lHqIesS5ZW5yDrTY4M9J5TLgocIOfhIri0rlyi"
#define BLYNK_TEMPLATE_ID "TMPL6OEQkrXDq"
#define BLYNK_TEMPLATE_NAME "AI Careon"
#define BLYNK_AUTH_TOKEN "AE3oiPNaE0OxUMmlLEfDjjqicigDZr6-"
#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <BlynkSimpleEsp32.h>
#include <ArduinoJson.h>
#include "config.h"
#include "sensors.h"
#include "actuators.h"

// ── Firebase Objects ────────────────────────────────────
FirebaseData   fbData;
FirebaseConfig fbConfig;
FirebaseAuth   fbAuth;

// ── WiFi Connection ─────────────────────────────────────
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("[WiFi] Connecting");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" Connected!");
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(" FAILED — continuing offline");
  }
}

// ── Firebase Init ───────────────────────────────────────
void initFirebase() {
  fbConfig.host                          = FIREBASE_HOST;
  fbConfig.signer.tokens.legacy_token   = FIREBASE_AUTH;
  Firebase.begin(&fbConfig, &fbAuth);
  Firebase.reconnectWiFi(true);
  Serial.println("[Firebase] Connected");
}

// ── Blynk Init ──────────────────────────────────────────
void initBlynk() {
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
  Serial.println("[Blynk] Connected");
}

// ── Upload to Firebase ──────────────────────────────────
void uploadToFirebase(SensorData& d, int risk_score,
                      unsigned long pressure_dur) {
  String path = "/patients/patient_001/latest";

  Firebase.setInt(fbData,   path + "/fsr_raw",          d.fsr_raw);
  Firebase.setFloat(fbData, path + "/temperature",       d.temperature);
  Firebase.setFloat(fbData, path + "/humidity",          d.humidity);
  Firebase.setFloat(fbData, path + "/weight_kg",         d.weight_kg);
  Firebase.setInt(fbData,   path + "/heart_rate",        d.heart_rate);
  Firebase.setFloat(fbData, path + "/spo2",              d.spo2);
  Firebase.setBool(fbData,  path + "/moisture_wet",      d.moisture_wet);
  Firebase.setInt(fbData,   path + "/risk_score",        risk_score);
  Firebase.setInt(fbData,   path + "/pressure_duration", (int)pressure_dur);
  Firebase.setInt(fbData,   path + "/timestamp",         (int)(millis()/1000));

  // Also log to history
  String hist = "/patients/patient_001/history/" + String(millis()/1000);
  Firebase.setInt(fbData,   hist + "/fsr_raw",           d.fsr_raw);
  Firebase.setInt(fbData,   hist + "/risk_score",        risk_score);
  Firebase.setFloat(fbData, hist + "/temperature",       d.temperature);

if (fbData.errorReason() != "") {
    Serial.print("[Firebase] Error: ");
    Serial.println(fbData.errorReason());
} else {
    Serial.println("[Firebase] Data uploaded OK");
}
}

// ── Send to Blynk App ───────────────────────────────────
void sendToBlynk(SensorData& d, int score,
                 unsigned long pressure_dur) {
  Blynk.virtualWrite(V0, d.fsr_raw);
  Blynk.virtualWrite(V1, d.temperature);
  Blynk.virtualWrite(V2, d.humidity);
  Blynk.virtualWrite(V3, d.heart_rate);
  Blynk.virtualWrite(V4, d.spo2);
  Blynk.virtualWrite(V5, score);
  Blynk.virtualWrite(V6, d.moisture_wet ? 255 : 0);
  Blynk.virtualWrite(V7, (int)pressure_dur);
}

// ── Send Push Alert ─────────────────────────────────────
void sendAlert(int risk_level, int score) {
  if (risk_level >= 2) {
    String msg = "Risk Score: " + String(score) + "/100";
    if (risk_level == 3) msg = "CRITICAL! " + msg;
    else                 msg = "DANGER — " + msg;
    Blynk.logEvent("pressure_alert", msg);
    Serial.print("[Blynk] Alert sent: ");
    Serial.println(msg);
  }
}

// ── Handle Manual Bed Control from Nurse's Phone ────────
BLYNK_WRITE(V9) {
  int btn = param.asInt();
  if (btn == 1) raiseBed();
  else          lowerBed();
}
