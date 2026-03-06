#pragma once

#include <stdint.h>
#include "config.h"
#include "maze.h"

void floodFill();
Direction getBestDirection(uint8_t x, uint8_t y, Direction heading);
