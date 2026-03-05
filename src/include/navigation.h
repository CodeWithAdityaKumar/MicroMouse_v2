#pragma once

#include "config.h"

Direction getNextDirection(uint8_t x, uint8_t y);
void turnToFace(Direction target, Direction current);
void computeSpeedRunPath();
void executeSpeedRun();
