// Test: Stage 6 — Exploration Mode
// This is the first PHYSICAL integration test. The robot will:
//   1. Start at (0,0) facing NORTH
//   2. On each step: read sensors, update maze, flood fill, choose dir, turn, move
//   3. Stop when it reaches the goal zone (center 4 cells)
// Serial Monitor will show coordinates and maze state after each step.

#include "config.h"
#include "motion.h"
#include "imu.h"
#include "motors.h"
#include "encoders.h"
#include "sensors.h"
#include "maze.h"
#include "navigation.h"
#include <Arduino.h>

// ── Button state machine ─────────────────────────────────────────────────────
enum TestState { TST_IDLE, TST_CALIBRATING, TST_READY, TST_EXPLORING, TST_DONE };
TestState state = TST_IDLE;

unsigned long buttonPressTime = 0;
bool buttonHeld = false;

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

  Serial.println(F("=== Stage 6: Exploration Mode ==="));
  Serial.println(F("Place robot at maze position (0,0) facing NORTH."));
  Serial.println(F("Click button to calibrate IMU."));
}

void loop() {
  // ── Long-press = reset ─────────────────────────────────────────────────────
  if (checkLongPress()) {
    Serial.println(F("\n[RESET] Returning to IDLE."));
    state = TST_IDLE;
    brake();
    delay(500);
    buttonHeld = false;
    buttonPressTime = 0;
    Serial.println(F("Click button to calibrate IMU."));
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
      
      // Initialize at (7,0) facing NORTH for right-side start.
      // Modify to (0,0, NORTH) if starting from the left side later.
      initNavigation(7, 0, NORTH);
      
      Serial.println(F("Ready! Click button to START EXPLORATION."));
      state = TST_READY;
      break;

    case TST_READY:
      if (digitalRead(BUTTON_PIN) == LOW && !buttonHeld) {
        waitForRelease();
        delay(500); // Small pause before launch
        Serial.println(F("Starting exploration..."));
        state = TST_EXPLORING;
      }
      break;

    case TST_EXPLORING: {
      // Log current position before moving
      Serial.print(F("Pos: ("));
      Serial.print(botX); Serial.print(F(","));
      Serial.print(botY); Serial.print(F(") Dir: "));
      const char* dNames[] = { "N", "E", "S", "W" };
      Serial.println(dNames[botDir]);

      // Execute one explore step
      bool atGoal = exploreStep();

      if (atGoal) {
        Serial.println(F("\n*** GOAL REACHED! ***"));
        Serial.print(F("Final position: ("));
        Serial.print(botX); Serial.print(F(","));
        Serial.print(botY); Serial.println(F(")"));
        Serial.println(F("Hold button to reset and run again."));
        
        // Print final maze state
        printMaze();
        
        state = TST_DONE;
      }
      break;
    }

    case TST_DONE:
      // Wait for long-press reset
      break;
  }
}
