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
void initNavigation() {
  botX  = 0;
  botY  = 0;
  botDir = NORTH;

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
  // 1. Read all 3 ultrasonic sensors
  updateSensors();

  // 2. Record walls detected from current cell/heading
  updateWalls(botX, botY, botDir, wallFront(), wallLeft(), wallRight());

  // 3. Mark cell as visited
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
