#include "include/motion.h"
#include "include/motors.h"
#include "include/encoders.h"
#include "include/imu.h"
#include "include/sensors.h"
#include <Arduino.h>

void initMotion() {
  initMotors();
  initEncoders();
  // IMU should be initialized separately in setup() 
  // because calibration requires the bot to be still.
}

void moveForward(float distance_cm, bool hasLeftWall, bool hasRightWall) {
  // 1 cell = 16 cm ≈ 532 ticks
  long targetTicks = (distance_cm * 10.0) / MM_PER_TICK;
  
  resetEncoders();
  
  long startL = getLeftTicks();
  long startR = getRightTicks();
  
  // Capture our target heading before we start moving
  updateYawAngle();
  float baseYaw = getYawAngle();
  
  // For derivative calculation in wall PID
  float lastLeftError = 0;
  float lastRightError = 0;
  unsigned long lastTime = millis();
  
  while (true) {
    long currentL = getLeftTicks() - startL;
    long currentR = getRightTicks() - startR;
    long avgTicks = (currentL + currentR) / 2;
    
    // Always poll IMU to maintain accuracy
    updateYawAngle();
    
    // Poll ALL sensors non-stop now that sensors.cpp handles the 15ms non-blocking Round-Robin limit
    updateSensors();
    
    if (avgTicks >= targetTicks) {
      break;
    }
    
    // --- Emergency Front Wall Stop ---
    // If we're closer than WALL_DANGER_CM, brake!
    if (distFront > 0 && distFront <= WALL_DANGER_CM) {
      break;
    }
    
    // --- 1. IMU Heading Error ---
    // (If yaw is positive/left of target, error is positive -> Steer right)
    float yawError = baseYaw - getYawAngle();
    
    // --- 2. Wall Centering Error (Proportional-Derivative) ---
    float leftWallError = 0;
    float rightWallError = 0;
    
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f; // in seconds
    if (dt <= 0) dt = 0.001;
    
    // Only apply wall centering if a wall is actually supposed to exist there
    if (hasLeftWall && distLeft > 0 && distLeft < WALL_CLEAR_CM) {
        float error = WALL_CENTER_CM - distLeft; // Target 3.5cm 
        if (abs(error) > 0.5) { // Deadband of 0.5cm to prevent micro-oscillations
          float derivative = (error - lastLeftError) / dt;
          leftWallError = error + (WALL_KD / WALL_KP) * derivative;
          lastLeftError = error;
        }
    }
    
    if (hasRightWall && distRight > 0 && distRight < WALL_CLEAR_CM) {
        float error = WALL_CENTER_CM - distRight;
        if (abs(error) > 0.5) {
          float derivative = (error - lastRightError) / dt;
          // Note: Error is subtracted, so steering pulls towards right wall
          rightWallError = error + (WALL_KD / WALL_KP) * derivative;
          lastRightError = error;
        }
    }
    
    lastTime = now;
    
    // Calculate total correction
    // Negative error = veer left to fix
    // Positive error = veer right to fix
    float totalCorrection = (yawError * STRAIGHT_KP) 
                            - (leftWallError * WALL_KP) 
                            + (rightWallError * WALL_KP);
                            
    // Cap outer correction boundary to prevent flipping out
    if (totalCorrection > 20) totalCorrection = 20;
    if (totalCorrection < -20) totalCorrection = -20;
    
    int leftPWM = BASE_SPEED + totalCorrection;
    int rightPWM = BASE_SPEED - totalCorrection;
    
    setMotors(leftPWM, rightPWM);
  }
  
  brake();
  delay(10); // let momentum settle slightly before moving on
}

void turnRight90() {
  updateYawAngle();
  float targetYaw = getYawAngle() - 90.0;
  
  while (true) {
    updateYawAngle();
    float currentYaw = getYawAngle();
    float error = currentYaw - targetYaw;
    
    // Optional: normalize error to [-180, 180]
    while (error > 180.0) error -= 360.0;
    while (error < -180.0) error += 360.0;
    
    // If we're within 1.0 degree of target, done
    if (abs(error) < 1.0) {
      break;
    }
    
    // P-control for turn speed (with a minimum floor to overcome friction)
    int turnSpeed = abs(error) * TURN_KP;
    if (turnSpeed < MIN_TURN_SPEED) turnSpeed = MIN_TURN_SPEED;
    if (turnSpeed > MAX_TURN_SPEED) turnSpeed = MAX_TURN_SPEED;
    
    // Turning right: left motor forward, right motor backward
    setMotors(turnSpeed, -turnSpeed);
  }
  brake();
}

void turnLeft90() {
  updateYawAngle();
  float targetYaw = getYawAngle() + 90.0;
  
  while (true) {
    updateYawAngle();
    float currentYaw = getYawAngle();
    float error = targetYaw - currentYaw;
    
    while (error > 180.0) error -= 360.0;
    while (error < -180.0) error += 360.0;
    
    if (abs(error) < 1.0) {
      break;
    }
    
    int turnSpeed = abs(error) * TURN_KP;
    if (turnSpeed < MIN_TURN_SPEED) turnSpeed = MIN_TURN_SPEED;
    if (turnSpeed > MAX_TURN_SPEED) turnSpeed = MAX_TURN_SPEED;
    
    // Turning left: left motor backward, right motor forward
    setMotors(-turnSpeed, turnSpeed);
  }
  brake();
}

void turnBack() {
  updateYawAngle();
  // 180 degree left turn
  float targetYaw = getYawAngle() + 180.0;
  
  while (true) {
    updateYawAngle();
    float currentYaw = getYawAngle();
    float error = targetYaw - currentYaw;
    
    while (error > 180.0) error -= 360.0;
    while (error < -180.0) error += 360.0;
    
    if (abs(error) < 1.0) {
      break;
    }
    
    int turnSpeed = abs(error) * TURN_KP;
    if (turnSpeed < MIN_TURN_SPEED) turnSpeed = MIN_TURN_SPEED;
    if (turnSpeed > MAX_TURN_SPEED) turnSpeed = MAX_TURN_SPEED;
    
    // Turning left around: left motor backward, right motor forward
    setMotors(-turnSpeed, turnSpeed);
  }
  brake();
}
