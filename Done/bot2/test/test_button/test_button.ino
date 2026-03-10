// Test: Push button with debounce
// Stage 1 — Step 2.6

#include "../../src/config.h"

static bool lastButtonState = HIGH;
static bool currentState = HIGH;
static unsigned long lastDebounceTime = 0;
static int pressCount = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("Push Button Test"));
  Serial.println(F("D13 → Button → GND (INPUT_PULLUP)"));
  Serial.println(F("Press the button — each press increments counter.\n"));

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
    if (reading != currentState) {
      currentState = reading;
      if (currentState == LOW) {
        // Button pressed
        pressCount++;
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.print(F("PRESS #"));
        Serial.println(pressCount);
      } else {
        // Button released
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println(F("  released"));
      }
    }
  }

  lastButtonState = reading;
}
