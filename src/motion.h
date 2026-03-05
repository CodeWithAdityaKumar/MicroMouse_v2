#pragma once

#include "config.h"

// Initialize motion dependencies (if any specific state is needed)
void initMotion();

// High level movement functions
void moveForward(float distance_cm);
void turnRight90();
void turnLeft90();
void turnBack();

// Helpers to get IMU and Encoders for PID feedback loops
void updateMotionState();
