#include "include/maze.h"
#include <Arduino.h>

Cell maze[MAZE_SIZE][MAZE_SIZE];

void initMaze() {
  // TODO: Clear all cells, set outer boundary walls
}

void updateWalls(uint8_t x, uint8_t y, Direction heading,
                 bool wF, bool wL, bool wR) {
  // TODO: Map sensor readings to absolute walls + mirror to neighbors
}

bool isGoal(uint8_t x, uint8_t y) {
  return (x == GOAL_X1 || x == GOAL_X2) &&
         (y == GOAL_Y1 || y == GOAL_Y2);
}

void printMaze() {
  // TODO: Print maze grid with walls and distances to Serial
}
