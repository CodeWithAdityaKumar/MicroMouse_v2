// Test: MPU6050 yaw angle
// Stage 1 — Step 2.3

#include <Wire.h>

#define MPU_ADDR 0x68

float yawAngle = 0.0;
float gyroBias = 0.0;
unsigned long lastTime = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("MPU6050 Yaw Test"));

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
    sum += readRawGyroZ();
    delay(5);
  }
  gyroBias = sum / 400.0;

  Serial.print(F("Bias: ")); Serial.println(gyroBias);
  Serial.println(F("Rotate robot. Yaw angle will print."));

  lastTime = micros();
}

int16_t readRawGyroZ() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x47); // GYRO_ZOUT_H
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2);
  int16_t val = Wire.read() << 8 | Wire.read();
  return val;
}

void loop() {
  unsigned long now = micros();
  float dt = (now - lastTime) / 1000000.0;
  lastTime = now;

  int16_t raw = readRawGyroZ();
  float gyroZ = (raw - gyroBias) / 131.0; // ±250°/s sensitivity
  yawAngle += gyroZ * dt;

  // Wrap to ±180
  if (yawAngle > 180.0) yawAngle -= 360.0;
  if (yawAngle < -180.0) yawAngle += 360.0;

  Serial.print(F("Yaw: ")); Serial.print(yawAngle, 1);
  Serial.println(F("°"));

  delay(5);
}
