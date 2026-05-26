# test_algorithms.py
# Tests all AI Careon algorithms with fake patient data

import time

# ── Moving Average ───────────────────────────────────────
def moving_average(history, new_val, window=10):
    history.append(new_val)
    if len(history) > window:
        history.pop(0)
    return sum(history) / len(history)

# ── Decision Tree ────────────────────────────────────────
def decision_tree(fsr, temp, hum, moisture, immobile, duration):
    score = 0
    if   fsr > 3500: score += 40
    elif fsr > 2800: score += 25
    elif fsr > 1800: score += 10

    if   duration > 7200: score += 25
    elif duration > 3600: score += 15
    elif duration > 1800: score += 8

    if immobile:     score += 20
    if temp  > 39.0: score += 10
    elif temp > 37.5: score += 5
    if hum   > 80.0: score += 10
    elif hum > 70.0: score += 5
    if moisture:     score += 5

    return min(score, 100)

def risk_label(score):
    if   score >= 80: return "🔴 CRITICAL"
    elif score >= 60: return "🟠 DANGER"
    elif score >= 40: return "🟡 CAUTION"
    else:             return "🟢 SAFE"

# ── Run Test Cases ───────────────────────────────────────
print("=" * 50)
print("   AI CAREON — Algorithm Test Suite")
print("=" * 50)

test_cases = [
    {
        "name"    : "Healthy patient — no risk",
        "fsr"     : 800,
        "temp"    : 36.5,
        "hum"     : 50.0,
        "moisture": False,
        "immobile": False,
        "duration": 0,
        "expected": "SAFE"
    },
    {
        "name"    : "High pressure — short time",
        "fsr"     : 3000,
        "temp"    : 36.8,
        "hum"     : 55.0,
        "moisture": False,
        "immobile": False,
        "duration": 600,
        "expected": "CAUTION"
    },
    {
        "name"    : "High pressure + no movement",
        "fsr"     : 3200,
        "temp"    : 37.8,
        "hum"     : 68.0,
        "moisture": False,
        "immobile": True,
        "duration": 2000,
        "expected": "DANGER"
    },
    {
        "name"    : "Critical — all factors combined",
        "fsr"     : 3800,
        "temp"    : 38.9,
        "hum"     : 75.0,
        "moisture": True,
        "immobile": True,
        "duration": 7500,
        "expected": "CRITICAL"
    },
    {
        "name"    : "Low pressure but very long duration",
        "fsr"     : 2000,
        "temp"    : 37.0,
        "hum"     : 60.0,
        "moisture": False,
        "immobile": True,
        "duration": 8000,
        "expected": "DANGER"
    },
]

passed = 0
failed = 0

for tc in test_cases:
    score  = decision_tree(
        tc["fsr"], tc["temp"], tc["hum"],
        tc["moisture"], tc["immobile"], tc["duration"]
    )
    label  = risk_label(score)
    result = "✅ PASS" if tc["expected"] in label else "❌ FAIL"
    if "PASS" in result: passed += 1
    else:                failed += 1

    print(f"\nTest: {tc['name']}")
    print(f"  Score    : {score}/100")
    print(f"  Result   : {label}")
    print(f"  Expected : {tc['expected']}")
    print(f"  Status   : {result}")

print()
print("=" * 50)
print(f"  Results: {passed} passed / {failed} failed")
print("=" * 50)