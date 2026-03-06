#include "include/maze.h"
#include <Arduino.h>

Cell maze[MAZE_SIZE][MAZE_SIZE];

// ─────────────────────────────────────────────────────────────────────────────
// initMaze: Reset all cells and stamp in the outer perimeter walls
// ─────────────────────────────────────────────────────────────────────────────
void initMaze() {
  for (uint8_t x = 0; x < MAZE_SIZE; x++) {
    for (uint8_t y = 0; y < MAZE_SIZE; y++) {
      maze[x][y].walls    = 0;
      maze[x][y].distance = 255; // Unknown / infinity
      maze[x][y].visited  = false;
    }
  }
  
  // Stamp outer perimeter walls
  for (uint8_t i = 0; i < MAZE_SIZE; i++) {
    maze[i][0].walls          |= WALL_S; // Bottom row: South wall
    maze[i][MAZE_SIZE-1].walls |= WALL_N; // Top row: North wall
    maze[0][i].walls          |= WALL_W; // Left col: West wall
    maze[MAZE_SIZE-1][i].walls |= WALL_E; // Right col: East wall
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// updateWalls: Translate relative sensor readings to absolute wall bitmasks.
//   heading = robot's absolute compass direction (NORTH=0, EAST=1, SOUTH=2, WEST=3)
//   wF = front wall seen, wL = left wall seen, wR = right wall seen
// ─────────────────────────────────────────────────────────────────────────────
void updateWalls(uint8_t x, uint8_t y, Direction heading,
                 bool wF, bool wL, bool wR) {
  
  // Map (heading + relative offset) to absolute direction bitmask
  // Offset: 0=Front, 3=Left(-1=+3), 1=Right
  auto absBit = [](Direction h, int offset) -> uint8_t {
    int dir = ((int)h + offset) % 4;
    switch (dir) {
      case 0: return WALL_N;
      case 1: return WALL_E;
      case 2: return WALL_S;
      case 3: return WALL_W;
    }
    return 0;
  };
  
  // Absolute bitmasks for each relative sensor
  uint8_t frontBit = absBit(heading, 0);
  uint8_t rightBit = absBit(heading, 1);
  uint8_t leftBit  = absBit(heading, 3);
  
  if (wF) maze[x][y].walls |= frontBit;
  if (wR) maze[x][y].walls |= rightBit;
  if (wL) maze[x][y].walls |= leftBit;
  
  // ── Mirror to neighbors ────────────────────────────────────────────────────  
  // Each wall is shared between two adjacent cells. We must update both.
  // Neighbor offsets: N=(0,+1), E=(+1,0), S=(0,-1), W=(-1,0)
  auto mirrorWall = [&](uint8_t bit) {
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
  };
  
  if (wF) mirrorWall(frontBit);
  if (wR) mirrorWall(rightBit);
  if (wL) mirrorWall(leftBit);
  
  maze[x][y].visited = true;
}

bool isGoal(uint8_t x, uint8_t y) {
  return (x == GOAL_X1 || x == GOAL_X2) &&
         (y == GOAL_Y1 || y == GOAL_Y2);
}

// ─────────────────────────────────────────────────────────────────────────────
// printMaze: Renders an ASCII representation of the 8x8 grid to Serial.
//   Each cell is drawn with N/S/E/W walls and the distance number.
// ─────────────────────────────────────────────────────────────────────────────
void printMaze() {
  Serial.println(F("\n=== MAZE STATE ==="));
  
  // Print from North (top) down to South (bottom)
  for (int y = MAZE_SIZE - 1; y >= 0; y--) {
    
    // --- Top border of this row ---
    for (uint8_t x = 0; x < MAZE_SIZE; x++) {
      Serial.print((maze[x][y].walls & WALL_N) ? "+---" : "+   ");
    }
    Serial.println("+");
    
    // --- Cell content row ---
    for (uint8_t x = 0; x < MAZE_SIZE; x++) {
      // Print left wall
      Serial.print((maze[x][y].walls & WALL_W) ? "|" : " ");
      
      // Print distance or '??' if not yet calculated
      if (maze[x][y].distance < 255) {
        char buf[4];
        sprintf(buf, "%3u", maze[x][y].distance);
        Serial.print(buf);
      } else {
        Serial.print(" ? ");
      }
    }
    // Print rightmost wall
    Serial.print((maze[MAZE_SIZE-1][y].walls & WALL_E) ? "|" : " ");
    Serial.println();
  }
  
  // --- Bottom border of the last row ---
  for (uint8_t x = 0; x < MAZE_SIZE; x++) {
    Serial.print((maze[x][0].walls & WALL_S) ? "+---" : "+   ");
  }
  Serial.println("+");
  Serial.println();
}
