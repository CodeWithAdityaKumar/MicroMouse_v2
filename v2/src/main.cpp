// Test: Stage 2 Motion Control
// Tests moving 1 cell forward, turning exactly 90 degrees right, 90 left, and 180 degrees back.

#include "Arduino.h"
#include "config.h"
#include "motors.h"
#include "encoders.h"
#include "imu.h"
#include "motion.h"
#include <Wire.h>
// Test: Stage 2 Motion Control
// Tests moving 1 cell forward, turning exactly 90 degrees right, 90 left, and 180 degrees back.


// Simple state machine for testing
enum TestState {
  TST_WAIT_CALIBRATE,
  TST_CALIBRATING,
  TST_WAIT_TEST,
  TST_TESTING,
  TST_DONE
};

TestState currentState = TST_WAIT_CALIBRATE;

// Button reading helper
bool waitForButtonPress() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50); // debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
      // wait for release
      while (digitalRead(BUTTON_PIN) == LOW) { delay(10); } 
      delay(50);
      return true;
    }
  }
  return false;
}

// Long press checking helper
bool checkLongPress() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    long pressStart = millis();
    while (digitalRead(BUTTON_PIN) == LOW) {
      if (millis() - pressStart > 1000) { // 1 second hold
        // Wait for release
        while (digitalRead(BUTTON_PIN) == LOW) { delay(10); }
        return true;
      }
      delay(10);
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("=== Stage 2 Motion Test ==="));

  // Initialize all subsystems
  initMotion();
  
  if (!initIMU()) {
    Serial.println(F("MPU6050 init failed!"));
    while (1);
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  Serial.println(F("System Ready."));
  Serial.println(F("-> CLICK button to start IMU Calibration."));
}

void loop() {
  
  // Always check for reset if we are done
  if (currentState == TST_DONE) {
    if (checkLongPress()) {
       Serial.println(F("\n--- SYSTEM RESET ---"));
       Serial.println(F("-> CLICK button to start IMU Calibration."));
       currentState = TST_WAIT_CALIBRATE;
    }
    return;
  }

  switch (currentState) {
    
    case TST_WAIT_CALIBRATE:
      if (waitForButtonPress()) {
        currentState = TST_CALIBRATING;
      }
      break;

    case TST_CALIBRATING:
      Serial.println(F("Calibrating IMU... Keep still for 3s."));
      if (!calibrateGyro()) {
        Serial.println(F("IMU calibration failed - too much movement. Try again."));
        currentState = TST_WAIT_CALIBRATE; // Go back
      } else {
        Serial.println(F("IMU Calibrated."));
        Serial.println(F("-> CLICK button to START the test sequence."));
        currentState = TST_WAIT_TEST;
      }
      break;

    case TST_WAIT_TEST:
      if (waitForButtonPress()) {
        currentState = TST_TESTING;
      }
      break;

    case TST_TESTING:
      Serial.println(F("Starting sequence..."));

      // 1. Move 1 cell
      Serial.println(F("Moving forward 1 cell (16cm)..."));
      moveForward(16.0);
      delay(1000);

      // 2. Turn 90 Right
      Serial.println(F("Turning 90 degrees Right..."));
      turnRight90();
      delay(1000);

       // 1. Move 1 cell
      Serial.println(F("Moving forward 1 cell (16cm)..."));
      moveForward(16.0);
      delay(1000);

      // 2. Turn 90 Right
      Serial.println(F("Turning 90 degrees Right..."));
      turnRight90();
      delay(1000);

       // 1. Move 1 cell
      Serial.println(F("Moving forward 1 cell (16cm)..."));
      moveForward(16.0);
      delay(1000);

      // 3. Turn 90 Left
      Serial.println(F("Turning 90 degrees Left (returning to straight)..."));
      turnLeft90();
      delay(1000);

       // 1. Move 1 cell
      Serial.println(F("Moving forward 1 cell (16cm)..."));
      moveForward(16.0);
      delay(1000);

       // 1. Move 1 cell
      Serial.println(F("Moving forward 1 cell (16cm)..."));
      moveForward(16.0);
      delay(1000);

       // 3. Turn 90 Left
      Serial.println(F("Turning 90 degrees Left (returning to straight)..."));
      turnLeft90();
      delay(1000);

      Serial.println(F("Test Complete!"));
      Serial.println(F("-> LONG PRESS (hold 1 sec) to reset."));
      currentState = TST_DONE;
      break;
      
    case TST_DONE:
      // Handled at top of loop
      break;
  }
}
