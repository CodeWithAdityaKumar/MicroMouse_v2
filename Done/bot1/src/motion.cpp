#include "motion.h"
#include "motors.h"
#include "encoders.h"
#include "imu.h"
#include "sensors.h"
#include <Arduino.h>



void initMotion() {
  initMotors();
  initEncoders();
  // IMU should be initialized separately in setup() 
  // because calibration requires the bot to be still.
}

void moveForward(float distance_cm) {
  // 1 cell = 16 cm ≈ 532 ticks
  long targetTicks = (distance_cm * 10.0) / MM_PER_TICK;
  
  resetEncoders();
  
  long startL = getLeftTicks();
  long startR = getRightTicks();
  
  // Capture our target heading before we start moving
  updateYawAngle();
  float baseYaw = getYawAngle();
  long lastSensorUpdate = millis();
  
  while (true) {
    long currentL = getLeftTicks() - startL;
    long currentR = getRightTicks() - startR;
    long avgTicks = (currentL + currentR) / 2;
    
    // Always poll IMU to maintain accuracy
    updateYawAngle();
    
    // Poll ultrasonic sensors every 20ms so we don't stall the motor loop 
    // (reading all 3 is now fully concurrent and takes a max of ~6ms)
    if (millis() - lastSensorUpdate >= 20) {
      updateSensors();
      lastSensorUpdate = millis();
    }
    
    if (avgTicks >= targetTicks) {
      break;
    }
    
    // --- Emergency Front Wall Stop ---
    // If we're closer than WALL_DANGER_CM + 1.0 buffer (3cm total), stop moving forward!
    // if (distFront > 0 && distFront <= (WALL_DANGER_CM)) {
    //   break;
    // }
    
    // --- 1. Encoder Error ---
    // (If Left > Right, error is positive -> Left slows down, Right speeds up)
    long encError = currentL - currentR;
    
    // --- 2. IMU Heading Error ---
    // (If yaw is positive/left of target, error is positive -> Steer right)
    float yawError = baseYaw - getYawAngle();
    
    // --- 3. Wall Centering Error ---
    float wallError = 0;
    int wallsSeen = 0;
    
    if (distLeft < WALL_DETECT_CM && distLeft > 0) {
      // If we are getting too close to the left wall (e.g. 2.0cm), we need to steer RIGHT.
      // Steer right = Left motor speeds up, Right motor slows down.
      // In our code: speedL = BASE_SPEED - correction. 
      // Therefore, to speed up Left we need a NEGATIVE correction.
      // 2.0 - 3.5 = -1.5 (Negative!) -> This gives negative correction!
      wallError += (distLeft - WALL_CENTER_CM);
      wallsSeen++;
    }
    if (distRight < WALL_DETECT_CM && distRight > 0) {
      // If we are getting too close to the right wall (e.g. 2.0cm), we need to steer LEFT.
      // Steer left = Right motor speeds up, Left motor slows down.
      // In our code: speedL = BASE_SPEED - correction. 
      // Therefore, to slow down Left we need a POSITIVE correction.
      // 3.5 - 2.0 = +1.5 (Positive!) -> This gives positive correction!
      wallError += (WALL_CENTER_CM - distRight);
      wallsSeen++;
    }
    if (wallsSeen > 0) {
      wallError /= wallsSeen;
    }
    
    // --- Combine Errors into Total Correction ---
    // **CRITICAL FIX**: We removed `encError` (LeftTicks - RightTicks) from this 
    // calculation because forcing the wheels to cover the exact same distance mathematically 
    // prevents the robot from steering left/right to avoid walls! 
    // The IMU (yawError) is powerful enough to keep the heading straight.
    int correction = (yawError * STRAIGHT_KP * 3.0) 
                   + (wallError * WALL_KP * 45.0);
    
    // Compute motor speeds 
    int speedL = BASE_SPEED - correction;
    int speedR = BASE_SPEED + correction;
    
    // Constrain to reasonable limits so we don't stall or go crazy
    speedL = constrain(speedL, BASE_SPEED - 50, BASE_SPEED + 50);
    speedR = constrain(speedR, BASE_SPEED - 50, BASE_SPEED + 50);
    
    setMotors(speedL, speedR);
    delay(5); // Small delay to allow encoder interrupts to fire and avoid tight looping
  }
  
  brake();
  delay(100);
}

void turnRight90() {
  resetEncoders();
  long targetTicks = TICKS_PER_90DEG_R;
  
  // Start turning right (Left motor forward, Right motor backward)
  setMotors(BASE_SPEED, -BASE_SPEED);
  
  while (true) {
    // Both motors contribute to rotation. Average their absolute moved distance
    long currentL = abs(getLeftTicks());
    long currentR = abs(getRightTicks());
    long avgMoved = (currentL + currentR) / 2;
    
    if (avgMoved >= targetTicks) {
      break;
    }
    delay(5);
  }
  
  brake();
  delay(100);
}

void turnLeft90() {
  resetEncoders();
  long targetTicks = TICKS_PER_90DEG_L;
  
  // Start turning left (Left motor backward, Right motor forward)
  setMotors(-BASE_SPEED, BASE_SPEED);
  
  while (true) {
    long currentL = abs(getLeftTicks());
    long currentR = abs(getRightTicks());
    long avgMoved = (currentL + currentR) / 2;
    
    if (avgMoved >= targetTicks) {
      break;
    }
    delay(5);
  }
  
  brake();
  delay(100);
}

void turnBack() {
  resetEncoders();
  // Uses its own dedicated tuning constant
  long targetTicks = TICKS_PER_180DEG; 
  
  // Start turning right
  setMotors(BASE_SPEED, -BASE_SPEED);
  
  while (true) {
    long currentL = abs(getLeftTicks());
    long currentR = abs(getRightTicks());
    long avgMoved = (currentL + currentR) / 2;
    
    if (avgMoved >= targetTicks) {
      break;
    }
    delay(5);
  }
  
  brake();
  delay(100);
}
