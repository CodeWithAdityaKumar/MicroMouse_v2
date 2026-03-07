#include "include/navigation.h"
#include "include/maze.h"
#include "include/sensors.h"
#include "include/motion.h"
#include "include/floodfill.h"
#include <Arduino.h>

// ── Robot position state ─────────────────────────────────────────────────────
uint8_t   botX   = 0;
uint8_t   botY   = 0;
Direction botDir  = NORTH;

// Speed-run path (Stage 7)
#define MAX_PATH 128
static char   speedRunPath[MAX_PATH];
static uint8_t pathLength = 0;

// Neighbor offsets for N, E, S, W
static const int8_t dx[4] = {  0, +1,  0, -1 };
static const int8_t dy[4] = { +1,  0, -1,  0 };

// ─────────────────────────────────────────────────────────────────────────────
// initNavigation: reset bot state, maze, and compute first flood fill
// ─────────────────────────────────────────────────────────────────────────────
void initNavigation(uint8_t startX, uint8_t startY, Direction startHeading) {
  botX  = startX;
  botY  = startY;
  botDir = startHeading;

  initMaze();
  floodFill();
  pathLength = 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// turnToFace: Rotate the robot from `current` to `target` direction
// using the fewest turns (prefer right turn for equal cost)
// ─────────────────────────────────────────────────────────────────────────────
void turnToFace(Direction target, Direction current) {
  int diff = ((int)target - (int)current + 4) % 4;
  switch (diff) {
    case 0: break;                // No turn needed
    case 1: turnRight90(); break; // 90° clockwise
    case 2: turnBack();    break; // 180°
    case 3: turnLeft90();  break; // 90° counter-clockwise
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// exploreStep: one full iteration of the explore loop
//   Returns true when the goal has been reached
// ─────────────────────────────────────────────────────────────────────────────
bool exploreStep() {
  // 1. Wait for robot to fully settle after previous turn/move before reading sensors
  delay(150);

  // 2. Take 3 samples per sensor and use majority vote to reject noise.
  //    A single bad echo from cross-talk can't record a false wall this way.
  int frontCount = 0, leftCount = 0, rightCount = 0;
  for (int i = 0; i < 3; i++) {
    updateSensors();
    // Use MAZE_WALL_CM (7cm), NOT WALL_DETECT_CM (10cm).
    // At an intersection, far corridor walls read ~8-9cm — above 7cm so they
    // won't be counted as walls. Actual adjacent walls are ~3.5cm.
    if (distFront < MAZE_WALL_CM) frontCount++;
    if (distLeft  < MAZE_WALL_CM) leftCount++;
    if (distRight < MAZE_WALL_CM) rightCount++;
  }
  bool wFront = (frontCount >= 2);
  bool wLeft  = (leftCount  >= 2);
  bool wRight = (rightCount >= 2);

  // Log to Serial so user can verify sensor readings during debugging
  Serial.print(F("  Sensors: F="));
  Serial.print(distFront, 1);
  Serial.print(F(" L="));
  Serial.print(distLeft, 1);
  Serial.print(F(" R="));
  Serial.print(distRight, 1);
  Serial.print(F("  Walls: F="));
  Serial.print(wFront ? '1' : '0');
  Serial.print(F(" L="));
  Serial.print(wLeft  ? '1' : '0');
  Serial.print(F(" R="));
  Serial.println(wRight ? '1' : '0');

  // 3. Record walls detected from current cell/heading (using voted readings)
  updateWalls(botX, botY, botDir, wFront, wLeft, wRight);

  // 4. Mark cell as visited
  maze[botX][botY].visited = true;

  // 4. Check if we have reached the goal
  if (isGoal(botX, botY)) {
    return true;
  }

  // 5. Recalculate flood fill with new wall knowledge
  floodFill();

  // 6. Choose the best direction based on the updated distance map
  Direction nextDir = getBestDirection(botX, botY, botDir);

  // 7. Turn to face the target direction
  turnToFace(nextDir, botDir);
  botDir = nextDir;

  // 8. Move forward exactly one cell
  moveForward(CELL_SIZE_MM / 10.0f);

  // 9. Update position
  botX = (uint8_t)(botX + dx[botDir]);
  botY = (uint8_t)(botY + dy[botDir]);

  return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// computeSpeedRunPath / executeSpeedRun: Stage 7 stubs
// ─────────────────────────────────────────────────────────────────────────────
void computeSpeedRunPath() {
  // TODO: Stage 7 — Trace optimal path, store as F/L/R sequence
}

void executeSpeedRun() {
  // TODO: Stage 7 — Replay stored path at SPEED_RUN_SPEED
}
