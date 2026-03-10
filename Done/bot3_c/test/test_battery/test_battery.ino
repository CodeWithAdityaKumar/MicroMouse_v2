// Test: Battery voltage via voltage divider
// Stage 1 — Step 2.5

#include "../src/config.h"

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println(F("Battery Voltage Test"));
  Serial.println(F("Expected: ~11.1V for full 3S LiPo"));
}

void loop() {
  int raw = analogRead(BATTERY_PIN);
  float vAdc = raw * (5.0 / 1023.0);
  float vBattery = vAdc * V_DIVIDER_RATIO;

  Serial.print(F("ADC: ")); Serial.print(raw);
  Serial.print(F("  V_adc: ")); Serial.print(vAdc, 2);
  Serial.print(F("V  V_bat: ")); Serial.print(vBattery, 1);
  Serial.println(F("V"));

  if (vBattery < BATTERY_CRITICAL_V) {
    Serial.println(F("⚠️ BATTERY CRITICAL!"));
  } else if (vBattery < BATTERY_LOW_V) {
    Serial.println(F("⚠️ Battery low"));
  }

  delay(1000);
}
