// Test: Stage 3 Wall Centering
// This test evaluates the new Sensor Fusion inside moveForward().
// It drives the bot forward 100cm. During the drive, placing a hand near
// the left or right ultrasonic sensor should cause the bot to steer away 
// (aiming for exactly 3.5cm distance) while the IMU pulls its heading straight.

#include "../../src/include/config.h"
#include "../../src/include/motion.h"
#include "../../src/include/imu.h"
#include "../../src/include/motors.h"
#include "../../src/include/encoders.h"
#include "../../src/include/sensors.h"
#include <Arduino.h>

enum TestState {
  TST_IDLE,
  TST_CALIBRATING,
  TST_READY,
  TST_TESTING,
  TST_DONE
};

TestState currentState = TST_IDLE;
unsigned long buttonPressTime = 0;
bool buttonHeld = false;

// Helpers
void waitForButtonPress() {
  while (digitalRead(BUTTON_PIN) == HIGH) {
    delay(10);
  }
  delay(DEBOUNCE_MS);
  while (digitalRead(BUTTON_PIN) == LOW) {
    delay(10);
  }
  delay(DEBOUNCE_MS);
}

bool checkLongPress() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (buttonPressTime == 0) {
      buttonPressTime = millis();
    } else if (millis() - buttonPressTime > 1000) {
      buttonHeld = true;
      return true;
    }
  } else {
    if (buttonHeld) {
      buttonHeld = false;
      buttonPressTime = 0;
      return false; // Already handled
    }
    buttonPressTime = 0;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  initMotion();
  initSensors();
  
  Serial.println(F("=== Stage 3: Dynamic Wall Centering Test ==="));
  Serial.println(F("Click button to Calibrate IMU."));
}

void loop() {
  // Check for reset hold
  if (checkLongPress()) {
    Serial.println(F("\n[RESET] Holding button... Resetting to IDLE state."));
    currentState = TST_IDLE;
    brake();
    delay(1000); 
    Serial.println(F("Click button to Calibrate IMU."));
  }

  // State Machine
  switch (currentState) {
    case TST_IDLE:
      if (digitalRead(BUTTON_PIN) == LOW && !buttonHeld) {
        delay(DEBOUNCE_MS);
        while(digitalRead(BUTTON_PIN) == LOW);
        currentState = TST_CALIBRATING;
      }
      break;
      
    case TST_CALIBRATING:
      Serial.println(F("Calibrating IMU... Keep bot STILL!"));
      initIMU(); 
      Serial.println(F("Calibration done."));
      Serial.println(F("=> Click button to START CENTERING TEST (Drives 100cm)."));
      Serial.println(F("=> (Hold button anytime to reset)"));
      currentState = TST_READY;
      break;
      
    case TST_READY:
      if (digitalRead(BUTTON_PIN) == LOW && !buttonHeld) {
        delay(DEBOUNCE_MS);
        while(digitalRead(BUTTON_PIN) == LOW);
        currentState = TST_TESTING;
      }
      break;
      
    case TST_TESTING:
      Serial.println(F("Moving Forward 100cm with Wall Avoidance active..."));
      delay(500); // Small pause before launch
      
      // Moving a long distance so you have time to test sliding a hand near the sides
      moveForward(100.0);
      
      Serial.println(F("Done."));
      Serial.println(F("=> Hold button to reset and try again."));
      currentState = TST_DONE;
      break;
      
    case TST_DONE:
      // Do nothing, wait for long press reset
      break;
  }
}
