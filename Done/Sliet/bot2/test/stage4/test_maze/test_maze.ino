// Test: Stage 4 — Maze Representation
// This test simulates the robot updating the in-memory maze map based on 
// hardcoded sensor readings and prints an ASCII map of the 8x8 grid to Serial.
// No motors run — this validates purely the maze data logic.

#include "config.h"
#include "maze.h"
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.println(F("=== Stage 4: Maze Representation Test ==="));
  Serial.println(F("Initializing a clean 8x8 maze..."));
  
  // 1. Init maze: sets all distances to 255, adds outer perimeter walls.
  initMaze();
  
  Serial.println(F("Initial (empty) maze:"));
  printMaze();
  
  // ─── Simulate Robot Path ────────────────────────────────────────────────────
  // We'll pretend the robot started at (0,0) facing NORTH and found:
  //   - A wall to the LEFT (West) — already set by perimeter
  //   - NO wall directly FRONT
  //   - A wall to the RIGHT (East)
  Serial.println(F("Simulating: Bot at (0,0) facing NORTH"));
  Serial.println(F("  Sees: LEFT=wall, FRONT=clear, RIGHT=wall"));
  updateWalls(0, 0, NORTH, false, true, true); // wF=false, wL=true, wR=true

  // Robot moved forward one cell to (0,1) still facing NORTH
  Serial.println(F("Simulating: Bot at (0,1) facing NORTH"));
  Serial.println(F("  Sees: LEFT=wall, FRONT=wall, RIGHT=clear"));
  updateWalls(0, 1, NORTH, true, false, true);  // wF=true, wL=false, wR=true

  // Bot turns EAST, now at (0,1) facing EAST
  // (didn't move yet, just turned — sensors update in-place)
  Serial.println(F("Simulating: Bot at (0,1) facing EAST (after turn)"));
  Serial.println(F("  Sees: LEFT=wall(N, already set), FRONT=clear, RIGHT=clear"));
  updateWalls(0, 1, EAST, false, false, false);

  // Bot moves right (East) to (1,1). Facing EAST.
  Serial.println(F("Simulating: Bot at (1,1) facing EAST"));
  Serial.println(F("  Sees: LEFT=clear, FRONT=clear, RIGHT=wall"));
  updateWalls(1, 1, EAST, false, true, false);

  // Mark all visited cells' distance manually for testing visibility
  maze[0][0].distance = 0;
  maze[0][1].distance = 1;
  maze[1][1].distance = 2;
  
  // ─── Print Updated Maze ────────────────────────────────────────────────────
  Serial.println(F("\nUpdated maze after simulation:"));
  printMaze();
  
  // ─── Test: isGoal check ───────────────────────────────────────────────────
  Serial.println(F("Goal cell checks:"));
  Serial.print(F("  isGoal(3,3): ")); Serial.println(isGoal(3, 3) ? F("PASS") : F("FAIL"));
  Serial.print(F("  isGoal(4,4): ")); Serial.println(isGoal(4, 4) ? F("PASS") : F("FAIL"));
  Serial.print(F("  isGoal(0,0): ")); Serial.println(!isGoal(0, 0) ? F("PASS (not goal)") : F("FAIL"));
  
  Serial.println(F("\n=== Test Complete ==="));
}

void loop() {
  // Nothing to do — test is complete
}
