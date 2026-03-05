// Test: MPU6050 yaw angle + Ultrasonic Centering
#include <Arduino.h>
#include <Wire.h>
#include "config.h"

float yawAngle = 0.0;
float gyroBias = 0.0;
unsigned long lastTime = 0;

bool isRunning = false;
float targetYaw = 0.0;

// Function Prototypes
int16_t readRawGyro();
float readUltrasonic(int echoPin);
void setMotors(int leftSpeed, int rightSpeed);

void setup() {
  Serial.begin(115200);

  // Setup Motor Pins
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH); // Enable Motor Driver

  // Setup Ultrasonic Pins
  pinMode(US_TRIG, OUTPUT);
  pinMode(US_FRONT_ECHO, INPUT);
  pinMode(US_LEFT_ECHO, INPUT);
  pinMode(US_RIGHT_ECHO, INPUT);

  // Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1
  Wire.write(0x00); // Wake up
  Wire.endTransmission();

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x1B);
  Wire.write(0x00); // ±250°/s
  Wire.endTransmission();

  Serial.println(F("Calibrating... keep still for 2 seconds"));
  delay(500);

  long sum = 0;
  for (int i = 0; i < 400; i++) {
    sum += readRawGyro();
    delay(5);
  }
  gyroBias = sum / 400.0;
  Serial.println(F("Calibration done. Press button to run..."));
}

int16_t readRawGyro() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(GYRO_REG);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2);
  return Wire.read() << 8 | Wire.read();
}

float readUltrasonic(int echoPin) {
  digitalWrite(US_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(US_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout max
  if (duration == 0) return 999.0;               // Out of bounds
  return duration * 0.0343 / 2.0;                // Return distance in cm
}

void setMotors(int leftSpeed, int rightSpeed) {
  leftSpeed = constrain(leftSpeed, -120, 120);
  rightSpeed = constrain(rightSpeed, -120, 120);

  // Left Motor
  if (leftSpeed >= 0) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, leftSpeed);
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    analogWrite(PWMA, -leftSpeed);
  }

  // Right Motor
  if (rightSpeed >= 0) {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, rightSpeed);
  } else {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    analogWrite(PWMB, -rightSpeed);
  }
}

void loop() {
  unsigned long now = micros();
  float dt = (now - lastTime) / 1000000.0;
  lastTime = now;

  // 1. Maintain MPU yaw angle
  float gyroRate = (readRawGyro() - gyroBias) / GYRO_SENS; 
  yawAngle += gyroRate * dt;

  // Wrap to ±180
  if (yawAngle > 180.0) yawAngle -= 360.0;
  if (yawAngle < -180.0) yawAngle += 360.0;

  // Toggle run state using the pushbutton
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(DEBOUNCE_MS);
    while (digitalRead(BUTTON_PIN) == LOW);
    isRunning = !isRunning;
    if (isRunning) {
      yawAngle = 0.0;    // Reset yaw relative to starting orientation
      targetYaw = 0.0;
    } else {
      setMotors(0, 0);   // Stop bot completely
    }
  }

  if (!isRunning) return;

  // 2. Read Wall Distances
  float leftDist = readUltrasonic(US_LEFT_ECHO);
  float rightDist = readUltrasonic(US_RIGHT_ECHO);
  float frontDist = readUltrasonic(US_FRONT_ECHO);

  // Auto-stop if approaching a solid front wall
  if (frontDist < WALL_DANGER_CM) {
    setMotors(0, 0);
    isRunning = false;
    return;
  }

  // Calculate Base Errors
  float yawError = targetYaw - yawAngle;
  
  // Use a PID approach: combine MPU straightness with Ultrasonic Wall Centering
  // MPU makes sure the bot is parallel to the walls
  float correction = STRAIGHT_KP * yawError;

  // 3. Wall Error (Centering between Left/Right walls)
  bool leftWall = (leftDist < WALL_CLEAR_CM);
  bool rightWall = (rightDist < WALL_CLEAR_CM);
  float wallError = 0.0;

  if (leftWall && rightWall) {
    // Both sides detected: keep zero difference between sensors
    // Positive wallError = steer left, Negative wallError = steer right
    wallError = rightDist - leftDist; 
  } else if (leftWall) {
    // Wall only on left: push away if closer than center
    wallError = WALL_CENTER_CM - leftDist;
  } else if (rightWall) {
    // Wall only on right: push away if closer than center
    wallError = rightDist - WALL_CENTER_CM;
  }

  // Instead of just adding the wall error, we use the wall error to change our "target orientation" 
  // or add it heavily to the correction. 
  // MPU tries to keep it straight (yawAngle -> 0), Wall sensors try to shift it sideways.
  correction += (WALL_KP * wallError);

  // Apply motor settings
  // If correction is positive: left motor slows down, right motor speeds up (turns left)
  // If correction is negative: left motor speeds up, right motor slows down (turns right)
  int leftSpeed = BASE_SPEED - correction;
  int rightSpeed = BASE_SPEED + correction;

  setMotors(leftSpeed, rightSpeed);
}
