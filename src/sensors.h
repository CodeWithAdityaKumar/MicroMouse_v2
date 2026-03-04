#pragma once

#include "config.h"

void initSensors();
float readFrontDistance();
float readLeftDistance();
float readRightDistance();

bool wallFront();
bool wallLeft();
bool wallRight();
