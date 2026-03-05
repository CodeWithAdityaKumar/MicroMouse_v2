#pragma once

#include "config.h"

void initPID();
float computeStraightPID(long leftTicks, long rightTicks);
float computeTurnPID(float targetAngle, float currentAngle);
void resetPID();
