// Test: Stage 2 - Step 5 (Straight Line PID)
// Tests moving 1 cell forward while keeping the robot perfectly straight.

#include "../../src/include/config.h"
#include "../../src/include/motors.h"
#include "../../src/include/encoders.h"
#include "../../src/include/imu.h"
#include "../../src/include/motion.h"
#include <Wire.h>

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
  Serial.println(F("=== Stage 2: Step 5 (Straight Line PID) ==="));

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
        Serial.println(F("-> CLICK button to START the straight line test."));
        currentState = TST_WAIT_TEST;
      }
      break;

    case TST_WAIT_TEST:
      if (waitForButtonPress()) {
        currentState = TST_TESTING;
      }
      break;

    case TST_TESTING:
      Serial.println(F("\n--- Starting Straight Line Test ---"));
      
      // Moving 1 to 3 cells is good for tuning PID, let's test 2 cells to give it
      // enough distance for the PID to visibly kick in and correct the drift.
      Serial.println(F("Moving forward 2 cells (32cm) to test PID tuning..."));
      
      // The moveForward function in motion.cpp contains the Straight Line PID logic
      // It compares left and right encoder ticks and adjusts motors to keep 'error = 0'
      moveForward(32.0);
      delay(1000);

      Serial.println(F("--- Test Complete! ---"));
      Serial.println(F("If the bot veered left/right, increase 'STRAIGHT_KP' in config.h"));
      Serial.println(F("If the bot wiggled aggressively, decrease 'STRAIGHT_KP' in config.h"));
      Serial.println(F("-> LONG PRESS (hold 1 sec) to reset and try again."));
      currentState = TST_DONE;
      break;
      
    case TST_DONE:
      // Handled at top of loop
      break;
  }
}
