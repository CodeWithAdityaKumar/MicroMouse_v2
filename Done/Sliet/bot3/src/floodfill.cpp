#include "floodfill.h"
#include "maze.h"
#include <Arduino.h>


#define QUEUE_SIZE 128

// ─────────────────────────────────────────────────────────────────────────────
// Neighbor offsets and wall masks for N, E, S, W
// ─────────────────────────────────────────────────────────────────────────────
static const int8_t dx[4] = {  0, +1,  0, -1 };  // N, E, S, W
static const int8_t dy[4] = { +1,  0, -1,  0 };
static const uint8_t wallBit[4] = { WALL_N, WALL_E, WALL_S, WALL_W };

// ─────────────────────────────────────────────────────────────────────────────
// floodFill: BFS from all 4 goal cells outward.
//   Every reachable cell gets a distance = fewest open-wall hops to goal.
// ─────────────────────────────────────────────────────────────────────────────
void floodFill() {
  // 1. Reset all distances
  for (uint8_t x = 0; x < MAZE_SIZE; x++) {
    for (uint8_t y = 0; y < MAZE_SIZE; y++) {
      maze[x][y].distance = 255;
    }
  }

  // 2. BFS queue (fixed array — no heap allocation on Arduino)
  //    Each entry is packed as (x << 4 | y) for compactness (values 0-7 each)
  uint8_t queue[QUEUE_SIZE];
  uint8_t head = 0, tail = 0;

  // 3. Seed the 4 goal cells at distance 0
  const uint8_t goalX[] = { GOAL_X1, GOAL_X1, GOAL_X2, GOAL_X2 };
  const uint8_t goalY[] = { GOAL_Y1, GOAL_Y2, GOAL_Y1, GOAL_Y2 };

  for (uint8_t i = 0; i < 4; i++) {
    uint8_t gx = goalX[i], gy = goalY[i];
    maze[gx][gy].distance = 0;
    queue[tail++] = (gx << 4) | gy;
  }

  // 4. BFS propagation
  while (head != tail) {
    uint8_t cell = queue[head++];
    if (head >= QUEUE_SIZE) head = 0; // ring buffer wrap

    uint8_t cx = (cell >> 4) & 0x0F;
    uint8_t cy = cell & 0x0F;

    uint8_t nextDist = maze[cx][cy].distance + 1;

    for (uint8_t dir = 0; dir < 4; dir++) {
      // Skip if there is a wall between current cell and neighbor
      if (maze[cx][cy].walls & wallBit[dir]) continue;

      int8_t nx = cx + dx[dir];
      int8_t ny = cy + dy[dir];

      // Bounds check
      if (nx < 0 || nx >= MAZE_SIZE || ny < 0 || ny >= MAZE_SIZE) continue;

      // Only update if we found a shorter path
      if (maze[nx][ny].distance > nextDist) {
        maze[nx][ny].distance = nextDist;
        queue[tail++] = ((uint8_t)nx << 4) | (uint8_t)ny;
        if (tail >= QUEUE_SIZE) tail = 0; // ring buffer wrap
      }
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// getBestDirection: Find the open neighbor with the lowest flood fill distance.
//   Prefers straight ahead to minimize turns if there's a tie.
// ─────────────────────────────────────────────────────────────────────────────
Direction getBestDirection(uint8_t x, uint8_t y, Direction heading) {
  uint8_t   bestDist  = 255;
  Direction bestDir   = heading;
  bool      foundOpen = false;

  // Check all 4 directions, prioritizing the current heading for ties
  for (int8_t offset = 0; offset < 4; offset++) {
    // Order: 0=straight, 1=right, 3=left, 2=back
    int8_t prio[4] = { 0, 1, 3, 2 }; // offsets from heading, priority order
    Direction dir = (Direction)(((int)heading + prio[offset] + 4) % 4);

    // Check for wall
    if (maze[x][y].walls & wallBit[dir]) continue;

    int8_t nx = x + dx[dir];
    int8_t ny = y + dy[dir];

    if (nx < 0 || nx >= MAZE_SIZE || ny < 0 || ny >= MAZE_SIZE) continue;

    if (!foundOpen) {
      bestDir = dir; // First valid open direction as safe fallback
      foundOpen = true;
    }

    bool isBetter = false;
    if (maze[nx][ny].distance < bestDist) {
      isBetter = true;
    } else if (maze[nx][ny].distance == bestDist && bestDist < 255) {
      // Tie-breaker: prefer unvisited neighbor over already visited neighbor
      int8_t bx = x + dx[bestDir];
      int8_t by = y + dy[bestDir];
      if (maze[bx][by].visited && !maze[nx][ny].visited) {
        isBetter = true;
      }
    }

    if (isBetter) {
      bestDist = maze[nx][ny].distance;
      bestDir  = dir;
    }
  }

  return bestDir;
}
