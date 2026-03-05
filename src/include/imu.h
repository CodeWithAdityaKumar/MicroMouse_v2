#pragma once

#include "config.h"

bool initIMU();
bool calibrateGyro();
void updateYawAngle();
float getYawAngle();
void resetYaw();
