#include "navigation.h"
#include "maze.h"
#include "movement.h"

#define MAX_PATH 64

static char speedRunPath[MAX_PATH];
static uint8_t pathLength = 0;

Direction getNextDirection(uint8_t x, uint8_t y) {
  // TODO: Find accessible neighbor with lowest flood fill distance
  return NORTH;
}

void turnToFace(Direction target, Direction current) {
  // TODO: Calculate turn needed and execute
}

void computeSpeedRunPath() {
  // TODO: Trace optimal path, store as F/L/R sequence
}

void executeSpeedRun() {
  // TODO: Replay stored path at SPEED_RUN_SPEED
}
