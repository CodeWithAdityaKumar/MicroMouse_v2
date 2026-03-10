// Test: Stage 5 — Flood Fill Algorithm
// Validates the BFS flood fill on an empty 8x8 maze and one with a simulated wall.
// No motors run — purely a logic test on the in-memory maze.

#include "../../src/include/config.h"
#include "../../src/include/maze.h"
#include "../../src/include/floodfill.h"
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.println(F("=== Stage 5: Flood Fill Test ===\n"));

  // ─── Test 1: Empty maze ───────────────────────────────────────────────────
  Serial.println(F("--- Test 1: Empty maze (only perimeter walls) ---"));
  initMaze();
  floodFill();
  printMaze();

  // Spot-check distances
  Serial.println(F("Spot checks (empty maze):"));
  Serial.print(F("  maze[3][3].distance == 0: "));
  Serial.println(maze[3][3].distance == 0 ? F("PASS") : F("FAIL"));
  Serial.print(F("  maze[4][4].distance == 0: "));
  Serial.println(maze[4][4].distance == 0 ? F("PASS") : F("FAIL"));
  // In an 8x8 maze with goals at (3-4, 3-4), corner (0,0) is 3+3=6 BFS hops to (3,3)
  Serial.print(F("  maze[0][0].distance == 6: "));
  Serial.println(maze[0][0].distance == 6 ? F("PASS") : F("FAIL"));
  Serial.print(F("  maze[7][7].distance == 6: "));
  Serial.println(maze[7][7].distance == 6 ? F("PASS") : F("FAIL"));
  Serial.print(F("  maze[0][7].distance == 6: "));
  Serial.println(maze[0][7].distance == 6 ? F("PASS") : F("FAIL"));

  // ─── Test 2: Maze with a simulated wall ───────────────────────────────────
  Serial.println(F("\n--- Test 2: Wall added west of (4,3) and east of (3,3) ---"));
  Serial.println(F("Flood fill must route around it — expect +1 on west side."));
  initMaze();
  
  // Add a vertical wall barrier between column 3 and column 4 in the middle row
  maze[3][3].walls |= WALL_E;
  maze[4][3].walls |= WALL_W;
  maze[3][4].walls |= WALL_E;
  maze[4][4].walls |= WALL_W;

  floodFill();
  printMaze();

  // The wall is between (3,3)↔(4,3) and (3,4)↔(4,4) — splitting goal cells.
  // (2,3) can still reach goal (3,3) directly going East — no wall there.
  // So distance from (2,3) = 1. The wall only affects routing ACROSS it (e.g. x=4 side needs to go around)
  Serial.print(F("  maze[2][3].distance (should be 1): "));
  Serial.println(maze[2][3].distance == 1 ? F("PASS") : F("FAIL"));
  // On the OTHER side of the wall, (5,3) can reach goal (4,3) directly — distance 1.
  // But to reach (3,3), it would need to go around — so distance 1 still (nearest goal is (4,3)).
  Serial.print(F("  maze[5][3].distance (should be 1): "));
  Serial.println(maze[5][3].distance == 1 ? F("PASS") : F("FAIL"));
  
  // ─── Test 3: getBestDirection ─────────────────────────────────────────────
  Serial.println(F("\n--- Test 3: getBestDirection from (0,0) facing NORTH ---"));
  initMaze();
  floodFill();
  Direction best = getBestDirection(0, 0, NORTH);
  Serial.print(F("  Best dir from (0,0): "));
  const char* dNames[] = { "NORTH", "EAST", "SOUTH", "WEST" };
  Serial.println(dNames[best]);
  // In empty maze from corner (0,0) facing NORTH, robot should go NORTH or EAST (both dist=6)
  Serial.println((best == NORTH || best == EAST) ? F("  => PASS") : F("  => FAIL (unexpected dir)"));

  Serial.println(F("\n=== Test Complete ==="));
}

void loop() {
  // Nothing to do
}
