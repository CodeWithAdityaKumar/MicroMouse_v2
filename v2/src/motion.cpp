#include "motion.h"
#include "motors.h"
#include "encoders.h"
#include "imu.h"
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
  
  while (true) {
    long currentL = getLeftTicks() - startL;
    long currentR = getRightTicks() - startR;
    long avgTicks = (currentL + currentR) / 2;
    
    if (avgTicks >= targetTicks) {
      break;
    }
    
    // Straight line PID correction
    long error = currentL - currentR;
    int correction = error * STRAIGHT_KP;
    
    // Compute motor speeds (Left motor is naturally a bit faster/slower? PID fixes it)
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
  // 180 degrees is exactly two 90-degree turns
  long targetTicks = (TICKS_PER_90DEG_R) * 2; 
  
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
