#pragma once

#include "config.h"

struct Cell {
  uint8_t walls;     // Bitmask: bit 0=N, 1=E, 2=S, 3=W
  uint8_t distance;  // Flood fill distance value (0-255)
  bool visited;      // Has robot entered this cell?
};

extern Cell maze[MAZE_SIZE][MAZE_SIZE];

void initMaze();
void updateWalls(uint8_t x, uint8_t y, Direction heading,
                 bool wF, bool wL, bool wR);
bool isGoal(uint8_t x, uint8_t y);
void printMaze();
