#include "imu.h"
#include "config.h"
#include <Arduino.h>
#include <Wire.h>

static float yawAngle = 0.0;
static float gyroBias = 0.0;
static unsigned long lastIMUTime = 0;

static int16_t readRawGyro() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(GYRO_REG);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2);
  return Wire.read() << 8 | Wire.read();
}

bool initIMU() {
  Wire.begin();
  
  // Wake up MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1
  Wire.write(0x00);
  if (Wire.endTransmission() != 0) {
    return false; // MPU not found
  }

  // Set gyro range to ±250°/s
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B);
  Wire.write(0x00);
  Wire.endTransmission();

  return true;
}

bool calibrateGyro() {
  long sum = 0;
  // Discard first few readings
  for (int i = 0; i < 50; i++) {
    readRawGyro();
    delay(2);
  }
  
  // Average 400 samples
  for (int i = 0; i < 400; i++) {
    sum += readRawGyro();
    delay(5);
  }
  gyroBias = sum / 400.0;
  lastIMUTime = micros();
  return true;
}

void updateYawAngle() {
  unsigned long now = micros();
  float dt = (now - lastIMUTime) / 1000000.0;
  lastIMUTime = now;

  int16_t raw = readRawGyro();
  float rate = (raw - gyroBias) / GYRO_SENS;
  
  // Positive yaw = left, negative yaw = right
  yawAngle += rate * dt;

  // Wrap to +/- 180 degrees
  if (yawAngle > 180.0) yawAngle -= 360.0;
  if (yawAngle < -180.0) yawAngle += 360.0;
}

float getYawAngle() {
  return yawAngle;
}

void resetYaw() {
  yawAngle = 0.0;
}
