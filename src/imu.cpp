#include "imu.h"
#include <Wire.h>

static float yawAngle = 0.0;
static float gyroBias = 0.0;
static unsigned long lastIMUTime = 0;

bool initIMU() {
  // TODO: Initialize MPU6050 via I2C
  return false;
}

bool calibrateGyro() {
  // TODO: Average Z-axis samples, validate variance
  return false;
}

void updateYawAngle() {
  // TODO: Read gyro Z, subtract bias, integrate over dt
}

float getYawAngle() {
  return yawAngle;
}

void resetYaw() {
  yawAngle = 0.0;
}
