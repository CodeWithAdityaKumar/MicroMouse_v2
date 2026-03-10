// Test: Stage 7 — Explore and Speed Run
// This is the final integration test simulating a true Micromouse run.
// 1. Robot starts at (0,0) facing NORTH and explores the maze using floodfill.
// 2. Goal reached: Stops, prints maze and optimal path.
// 3. User physically moves robot back to (0,0) facing NORTH.
// 4. User clicks button. Robot executes speed run along optimal path.

#include "config.h"
#include "motion.h"
#include "imu.h"
#include "motors.h"
#include "encoders.h"
#include "sensors.h"
#include "maze.h"
#include "floodfill.h"
#include "navigation.h"
#include <Arduino.h>

enum TestState { TST_IDLE, TST_CALIBRATING, TST_EXPLORING, TST_SOLVED, TST_RUNNING, TST_DONE };
TestState state = TST_IDLE;

unsigned long buttonPressTime = 0;
bool buttonHeld = false;

// ── Button state machine ─────────────────────────────────────────────────────
bool checkLongPress() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (buttonPressTime == 0) buttonPressTime = millis();
    else if (millis() - buttonPressTime > 1000) { buttonHeld = true; return true; }
  } else {
    if (buttonHeld) { buttonHeld = false; buttonPressTime = 0; return false; }
    buttonPressTime = 0;
  }
  return false;
}

void waitForRelease() {
  while (digitalRead(BUTTON_PIN) == LOW) delay(10);
  delay(50);
}

// ─────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  initMotion();
  initSensors();

  Serial.println(F("=== Stage 7: Explore & Speed Run ==="));
  Serial.println(F("Place robot at maze position (0,0) facing NORTH."));
  Serial.println(F("Click button to calibrate IMU and start exploration."));
}

void loop() {
  // Long press reset
  if (checkLongPress()) {
    Serial.println(F("\n[RESET] Returning to IDLE."));
    state = TST_IDLE;
    brake();
    delay(500);
    buttonHeld = false;
    buttonPressTime = 0;
    Serial.println(F("Place at (0,0) NORTH. Click to Calibrate & Explore."));
    return;
  }

  switch (state) {
    case TST_IDLE:
      if (digitalRead(BUTTON_PIN) == LOW && !buttonHeld) {
        waitForRelease();
        state = TST_CALIBRATING;
      }
      break;

    case TST_CALIBRATING:
      Serial.println(F("Calibrating IMU — keep bot STILL..."));
      initIMU();
      
      Serial.println(F("Initializing Navigation..."));
      initNavigation(0, 0, NORTH);
      
      Serial.println(F("Starting exploration..."));
      delay(1000);
      state = TST_EXPLORING;
      break;

    case TST_EXPLORING: {
      bool atGoal = exploreStep();
      if (atGoal) {
        Serial.println(F("\n*** GOAL REACHED! ***"));
        printMaze();
        
        Serial.println(F("Computing Shortest Path (Speed Run Profile)..."));
        computeSpeedRunPath(); // Generates F/L/R string
        
        Serial.println(F("\n--- EXPLORATION COMPLETE ---"));
        Serial.println(F("1. Pick up the robot."));
        Serial.println(F("2. Place it back at START (0,0) facing NORTH."));
        Serial.println(F("3. Click button to EXECUTE Speed Run."));
        state = TST_SOLVED;
      }
      break;
    }

    case TST_SOLVED:
      if (digitalRead(BUTTON_PIN) == LOW && !buttonHeld) {
        waitForRelease();
        Serial.println(F("Recalibrating IMU for Speed Run... Keep STILL."));
        initIMU(); // Recalibrate in case the user tilted it when picking it up
        delay(1000); 
        state = TST_RUNNING;
      }
      break;

    case TST_RUNNING:
      executeSpeedRun();
      Serial.println(F("Hold button to reset."));
      state = TST_DONE;
      break;

    case TST_DONE:
      break;
  }
}
