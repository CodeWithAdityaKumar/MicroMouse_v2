// Test: Stage 7 — Speed Run Mode
// Simulates a known 8x8 maze, computes the shortest path using floodfill,
// and executes it at high speed with straight-line compression.

#include "../../src/include/config.h"
#include "../../src/include/motion.h"
#include "../../src/include/imu.h"
#include "../../src/include/motors.h"
#include "../../src/include/encoders.h"
#include "../../src/include/sensors.h"
#include "../../src/include/maze.h"
#include "../../src/include/floodfill.h"
#include "../../src/include/navigation.h"
#include <Arduino.h>

enum TestState { TST_IDLE, TST_CALIBRATING, TST_SOLVED, TST_RUNNING, TST_DONE };
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
void buildSimulatedMaze() {
  // We'll create a simple U-shape open path to the goal so the robot
  // has to drive forward, turn, drive, turn, and hit the center.
  // Goal is the 2x2 center: (3,3), (4,3), (3,4), (4,4)
  
  // To reach (4,3) from (0,0), let's make an open path:
  // (0,0) -> (0,3) -> (4,3)
  
  // We place a wall forcing it to go North to y=3, then East to x=4
  for(int x=1; x<=7; x++) {
    maze[x][0].walls |= WALL_W; // wall blocking immediate East travel from start
    mirrorWall(x, 0, WALL_W);
  }
}

void mirrorWall(int x, int y, uint8_t bit) {
    int nx = x, ny = y;
    uint8_t mirrorBit;
    switch (bit) {
      case WALL_N: ny++; mirrorBit = WALL_S; break;
      case WALL_S: ny--; mirrorBit = WALL_N; break;
      case WALL_E: nx++; mirrorBit = WALL_W; break;
      case WALL_W: nx--; mirrorBit = WALL_E; break;
      default: return;
    }
    if (nx >= 0 && nx < MAZE_SIZE && ny >= 0 && ny < MAZE_SIZE) {
      maze[nx][ny].walls |= mirrorBit;
    }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  initMotion();
  initSensors();
  initNavigation(0, 0, NORTH); // Start at standard (0,0)

  Serial.println(F("=== Stage 7: Speed Run Mode ==="));
  Serial.println(F("Place robot at maze position (0,0) facing NORTH."));
  Serial.println(F("Click button to calibrate IMU and solve path."));
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
      
      Serial.println(F("Building simulated mapped maze..."));
      initMaze();
      buildSimulatedMaze();
      
      // Calculate distances using Floodfill
      floodFill();
      printMaze();
      
      Serial.println(F("Computing Shortest Path..."));
      computeSpeedRunPath(); // Generates F/L/R string
      
      Serial.println(F("Ready! Click button to EXECUTE Speed Run."));
      state = TST_SOLVED;
      break;

    case TST_SOLVED:
      if (digitalRead(BUTTON_PIN) == LOW && !buttonHeld) {
        waitForRelease();
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
