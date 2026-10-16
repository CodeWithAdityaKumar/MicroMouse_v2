// Test: MPU6050 yaw angle
// Stage 1 — Step 2.3

#include <Wire.h>
#include "config.h"

float yawAngle = 0.0;
float gyroBias = 0.0;
unsigned long lastTime = 0;

int16_t readRawGyro();

void setup() {
  Serial.begin(115200);
  Serial.println(F("MPU6050 Yaw Test (X-axis)"));

  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1
  Wire.write(0x00); // Wake up
  Wire.endTransmission();

  // Set gyro range to ±250°/s
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B);
  Wire.write(0x00);
  Wire.endTransmission();

  Serial.println(F("Calibrating... keep still for 2 seconds"));
  delay(500);

  // Calibrate — average 400 samples
  long sum = 0;
  for (int i = 0; i < 400; i++) {
    sum += readRawGyro();
    delay(5);
  }
  gyroBias = sum / 400.0;

  Serial.print(F("Bias: ")); Serial.println(gyroBias);
  Serial.println(F("Rotate robot. Yaw angle will print."));

  lastTime = micros();
}

int16_t readRawGyro() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(GYRO_REG); // Defined in config.h (0x43 for X-axis)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2);
  int16_t val = Wire.read() << 8 | Wire.read();
  
  // Note: if left turn produces negative values, we can flip the sign here.
  // The user said: "when the bot is turning to left the value is in +ve and right the value is in -ve"
  // This is correct (left turn = +ve, right = -ve). We don't need to negate.
  return val;
}

void loop() {
  unsigned long now = micros();
  float dt = (now - lastTime) / 1000000.0;
  lastTime = now;

  int16_t raw = readRawGyro();
  float gyroRate = (raw - gyroBias) / GYRO_SENS; 
  yawAngle += gyroRate * dt;

  // Wrap to ±180
  if (yawAngle > 180.0) yawAngle -= 360.0;
  if (yawAngle < -180.0) yawAngle += 360.0;

  Serial.print(F("Yaw: ")); Serial.print(yawAngle, 1);
  Serial.println(F("°"));

  delay(5);
}
