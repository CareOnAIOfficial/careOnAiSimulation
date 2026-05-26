// ╔══════════════════════════════════════════════════════╗
// ║         AI CAREON — config.h                         ║
// ║         Credentials & Pin Definitions                ║
// ╚══════════════════════════════════════════════════════╝
#pragma once

// ── WiFi ────────────────────────────────────────────────
#define WIFI_SSID       "YourNetworkName"
#define WIFI_PASS       "YourPassword"

// ── Firebase ────────────────────────────────────────────
#define FIREBASE_HOST   "yourproject.firebaseio.com"
#define FIREBASE_AUTH   "your_firebase_secret_key"

// ── Blynk ───────────────────────────────────────────────
#define BLYNK_AUTH      "your_blynk_token"

// ── Pin Definitions (ESP32) ─────────────────────────────
#define PIN_DHT         4
#define PIN_FSR         34
#define PIN_MOISTURE    35
#define PIN_BTN_MOVE    32

#define PIN_LED_GREEN   13    // SAFE
#define PIN_LED_YELLOW  12    // CAUTION
#define PIN_LED_RED     27    // DANGER
#define PIN_LED_PUMP    25    // Air pump indicator
#define PIN_LED_VIB     26    // Vibration indicator
#define PIN_LED_ACT     33    // Linear actuator indicator
#define PIN_BUZZER      2

#define PIN_RELAY1      25    // Air pump 1
#define PIN_RELAY2      26    // Air pump 2 / solenoids
#define PIN_RELAY3      27    // Actuator direction A
#define PIN_RELAY4      14    // Actuator direction B

// ── I2C Pins (ESP32) ────────────────────────────────────
#define PIN_SDA         21
#define PIN_SCL         22

// ── Thresholds ──────────────────────────────────────────
#define PRESSURE_THRESHOLD    2800    // out of 4095 (12-bit ADC)
#define PRESSURE_DANGER       3500
#define PRESSURE_TIME_CAUTION 1800    // 30 minutes in seconds
#define PRESSURE_TIME_DANGER  3600    // 60 minutes in seconds
#define PRESSURE_TIME_LIMIT   7200    // 2 hours in seconds
#define TEMP_HIGH             37.5    // Celsius
#define HUMIDITY_HIGH         70.0    // Percent
#define SPO2_LOW              95.0    // Percent
#define MOISTURE_WET          2000    // out of 4095
#define NO_MOVE_LIMIT         5400    // 90 minutes in seconds
