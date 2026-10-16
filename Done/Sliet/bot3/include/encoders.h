#pragma once

#include "config.h"

extern volatile long leftTicks;
extern volatile long rightTicks;

void initEncoders();
void resetEncoders();
long getLeftTicks();
long getRightTicks();
