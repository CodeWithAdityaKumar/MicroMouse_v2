#include "pid.h"

static float prevError = 0;
static float integral = 0;

void initPID() {
  prevError = 0;
  integral = 0;
}

float computeStraightPID(long leftTicks, long rightTicks) {
  // TODO: Encoder differential PID
  return 0;
}

float computeTurnPID(float targetAngle, float currentAngle) {
  // TODO: Yaw-based turn correction
  return 0;
}

void resetPID() {
  prevError = 0;
  integral = 0;
}
