#include "motion.h"
#include "motors.h"
#include "encoders.h"
#include "imu.h"
#include "sensors.h"
#include <Arduino.h>



void initMotion() {
  initMotors();
  initEncoders();
  initSensors();
  // IMU should be initialized separately in setup() 
}

void moveForward(float distance_cm) {
  // 1 cell = 16 cm ≈ 602 ticks
  long targetTicks = (long)((distance_cm * 10.0) / MM_PER_TICK);
  
  resetEncoders();
  
  long startL = getLeftTicks();
  long startR = getRightTicks();
  
  // Capture our target heading before we start moving
  updateYawAngle();
  float baseYaw = getYawAngle();
  long lastSensorUpdate = millis();
  unsigned long startMoveTime = millis();
  // Safe timeout: ~1.2s per 16cm cell + 800ms buffer to prevent runaway
  unsigned long maxMoveTime = (unsigned long)((distance_cm / 16.0f) * 1200) + 800;
  
  while (true) {
    long currentL = abs(getLeftTicks() - startL);
    long currentR = abs(getRightTicks() - startR);
    long avgTicks = (currentL + currentR) / 2;
    
    // Always poll IMU to maintain accuracy
    updateYawAngle();
    
    // Poll ultrasonic sensors every 20ms
    if (millis() - lastSensorUpdate >= 20) {
      updateSensors();
      lastSensorUpdate = millis();
    }
    
    // Distance reached or safety timeout
    if (avgTicks >= targetTicks || (millis() - startMoveTime) > maxMoveTime) {
      break;
    }
    
    // Immediate Front Collision Stop:
    // If front wall is within 3.8cm, stop immediately regardless of ticks to prevent touching!
    if (distFront > 1.0 && distFront <= 3.8) {
      break;
    }

    // Normal cell arrival stop near front wall (between 4.0 and 5.5cm)
    if (avgTicks > (targetTicks * 0.60)) {
       if (distFront > 1.0 && distFront <= 5.5) {
         break;
       }
    }
    
    // --- 1. IMU Heading Error ---
    float yawError = baseYaw - getYawAngle();
    
    // --- 2. Wall Centering & Emergency Wall Repulsion ---
    float wallError = 0;
    int wallsSeen = 0;
    
    if (distLeft > 0 && distLeft < WALL_DETECT_CM) {
      if (distLeft <= WALL_DANGER_CM) {
        // Dangerously close to left wall! Strong push right!
        wallError -= 3.5;
        wallsSeen++;
      } else {
        wallError += (distLeft - WALL_CENTER_CM);
        wallsSeen++;
      }
    }
    if (distRight > 0 && distRight < WALL_DETECT_CM) {
      if (distRight <= WALL_DANGER_CM) {
        // Dangerously close to right wall! Strong push left!
        wallError += 3.5;
        wallsSeen++;
      } else {
        wallError += (WALL_CENTER_CM - distRight);
        wallsSeen++;
      }
    }
    if (wallsSeen > 0) {
      wallError /= wallsSeen;
    }
    
    // Combine Errors into Total Correction
    int correction = (yawError * STRAIGHT_KP * 1.5) 
                   + (wallError * WALL_KP * 20.0);
    
    // Decelerate if approaching front wall to prevent collision momentum
    int currentBase = BASE_SPEED;
    if (distFront > 1.0 && distFront <= 7.5) {
      currentBase = 45; // Smooth crawl
    }

    // Compute motor speeds 
    int speedL = currentBase - correction;
    int speedR = currentBase + correction;
    
    // Constrain to reasonable limits so we don't stall or go crazy
    speedL = constrain(speedL, currentBase - 45, currentBase + 45);
    speedR = constrain(speedR, currentBase - 45, currentBase + 45);
    
    setMotors(speedL, speedR);
    delay(5); // Small delay to allow encoder interrupts to fire and avoid tight looping
  }
  
  brake();
  delay(80);

  // Post-stop clearance: if too close to front wall (< 2.8cm), back off slightly
  updateSensors();
  if (distFront > 0.5 && distFront < 2.8) {
    setMotors(-MIN_TURN_SPEED, -MIN_TURN_SPEED);
    delay(35);
    brake();
    delay(50);
  }
}

void turnRight90() {
  resetEncoders();
  resetYaw();
  
  unsigned long startTurnTime = millis();
  const unsigned long TURN_TIMEOUT_MS = 1500;
  
  while (true) {
    updateYawAngle();
    float currentYaw = getYawAngle(); // Negative when turning right
    float degreesTurned = -currentYaw; // Positive progress (0 -> 90)
    
    // Fallback: encoder ticks
    long currentL = abs(getLeftTicks());
    long currentR = abs(getRightTicks());
    long avgMoved = (currentL + currentR) / 2;
    
    // Cut off at 83 deg to allow coasting to land cleanly at 90 deg
    if (degreesTurned >= 83.0f || avgMoved >= TICKS_PER_90DEG_R || (millis() - startTurnTime) > TURN_TIMEOUT_MS) {
      break;
    }
    
    // Smooth deceleration as target approaches
    int turnSpeed = BASE_SPEED;
    if (degreesTurned > 45.0f) {
      turnSpeed = map((long)degreesTurned, 45, 83, BASE_SPEED, MIN_TURN_SPEED);
      turnSpeed = constrain(turnSpeed, MIN_TURN_SPEED, BASE_SPEED);
    }
    
    // Right turn: Left motor forward, Right motor backward
    setMotors(turnSpeed, -turnSpeed);
    delay(5);
  }
  
  brake();
  delay(100);
  resetYaw();
}

void turnLeft90() {
  resetEncoders();
  resetYaw();
  
  unsigned long startTurnTime = millis();
  const unsigned long TURN_TIMEOUT_MS = 1500;
  
  while (true) {
    updateYawAngle();
    float currentYaw = getYawAngle(); // Positive when turning left
    float degreesTurned = currentYaw;  // Positive progress (0 -> 90)
    
    // Fallback: encoder ticks
    long currentL = abs(getLeftTicks());
    long currentR = abs(getRightTicks());
    long avgMoved = (currentL + currentR) / 2;
    
    // Cut off at 83 deg to allow coasting to land cleanly at 90 deg
    if (degreesTurned >= 83.0f || avgMoved >= TICKS_PER_90DEG_L || (millis() - startTurnTime) > TURN_TIMEOUT_MS) {
      break;
    }
    
    // Smooth deceleration as target approaches
    int turnSpeed = BASE_SPEED;
    if (degreesTurned > 45.0f) {
      turnSpeed = map((long)degreesTurned, 45, 83, BASE_SPEED, MIN_TURN_SPEED);
      turnSpeed = constrain(turnSpeed, MIN_TURN_SPEED, BASE_SPEED);
    }
    
    // Left turn: Left motor backward, Right motor forward
    setMotors(-turnSpeed, turnSpeed);
    delay(5);
  }
  
  brake();
  delay(100);
  resetYaw();
}

void turnBack() {
  resetEncoders();
  resetYaw();
  
  updateSensors();
  bool turnLeftDir = (distLeft > distRight);
  
  unsigned long startTurnTime = millis();
  const unsigned long TURN_TIMEOUT_MS = 2500;
  
  while (true) {
    updateYawAngle();
    float currentYaw = getYawAngle();
    float degreesTurned = turnLeftDir ? currentYaw : -currentYaw;
    if (degreesTurned < 0 && degreesTurned < -90.0f) degreesTurned += 360.0f; // wrap protection
    
    long currentL = abs(getLeftTicks());
    long currentR = abs(getRightTicks());
    long avgMoved = (currentL + currentR) / 2;
    
    // Cut off at 168 deg to allow coasting to land cleanly at 180 deg
    if (degreesTurned >= 168.0f || avgMoved >= TICKS_PER_180DEG || (millis() - startTurnTime) > TURN_TIMEOUT_MS) {
      break;
    }
    
    int turnSpeed = BASE_SPEED;
    if (degreesTurned > 100.0f) {
      turnSpeed = map((long)degreesTurned, 100, 168, BASE_SPEED, MIN_TURN_SPEED);
      turnSpeed = constrain(turnSpeed, MIN_TURN_SPEED, BASE_SPEED);
    }
    
    if (turnLeftDir) {
      setMotors(-turnSpeed, turnSpeed);
    } else {
      setMotors(turnSpeed, -turnSpeed);
    }
    delay(5);
  }
  
  brake();
  delay(100);
  resetYaw();
}
