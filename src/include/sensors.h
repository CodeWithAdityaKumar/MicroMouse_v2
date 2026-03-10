#pragma once

#include "config.h"

// Expose state variables for debugging/dashboarding
extern float distFront;
extern float distLeft;
extern float distRight;

extern bool isWallFront;
extern bool isWallLeft;
extern bool isWallRight;

void initSensors();

// Call this function continuously in the loop to get fresh data (Non-blocking)
void updateSensors();

// Call this when the robot is fully stopped to get a guaranteed full sweep (Blocking)
void readAllSensorsBlocking();

bool wallFront();
bool wallLeft();
bool wallRight();
