#include "sensors.h"
#include <Arduino.h>



// Global state variables
float distFront = 999.0;
float distLeft = 999.0;
float distRight = 999.0;

bool isWallFront = false;
bool isWallLeft = false;
bool isWallRight = false;

void initSensors() {
  pinMode(US_TRIG, OUTPUT);
  pinMode(US_FRONT_ECHO, INPUT);
  pinMode(US_LEFT_ECHO, INPUT);
  pinMode(US_RIGHT_ECHO, INPUT);
  
  digitalWrite(US_TRIG, LOW);
}

// We will use direct port manipulation on PINC (Analog inputs 0-5 on Nano)
// US_LEFT_ECHO  = A0 (Bit 0)
// US_RIGHT_ECHO = A1 (Bit 1)
// US_FRONT_ECHO = A3 (Bit 3)

void updateSensors() {
  // 1. Send 10us trigger pulse to all sensors simultaneously
  digitalWrite(US_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(US_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG, LOW);
  
  // 2. Prepare variables for concurrent pulse timing
  unsigned long startTime = micros();
  unsigned long timeout = 30000; // 30ms timeout (~5 meters)
  
  unsigned long startLeft = 0, startRight = 0, startFront = 0;
  unsigned long endLeft = 0, endRight = 0, endFront = 0;
  
  bool waitingLeft = true, waitingRight = true, waitingFront = true;
  bool doneLeft = false, doneRight = false, doneFront = false;
  
  // 3. Busy-wait loop: poll PINC continuously until all 3 echos complete or timeout
  while ((micros() - startTime) < timeout) {
    uint8_t currentPins = PINC; // Read all analog pins at once instantly
    unsigned long now = micros();
    
    // Process LEFT (A0 / Bit 0)
    if (!doneLeft) {
      if (waitingLeft) {
        if (currentPins & 0x01) { startLeft = now; waitingLeft = false; }
      } else {
        if (!(currentPins & 0x01)) { endLeft = now; doneLeft = true; }
      }
    }
    
    // Process RIGHT (A1 / Bit 1)
    if (!doneRight) {
      if (waitingRight) {
        if (currentPins & 0x02) { startRight = now; waitingRight = false; }
      } else {
        if (!(currentPins & 0x02)) { endRight = now; doneRight = true; }
      }
    }
    
    // Process FRONT (A3 / Bit 3)
    if (!doneFront) {
      if (waitingFront) {
        if (currentPins & 0x08) { startFront = now; waitingFront = false; }
      } else {
        if (!(currentPins & 0x08)) { endFront = now; doneFront = true; }
      }
    }
    
    // Exit early if all 3 sensors have received their falling edges
    if (doneLeft && doneRight && doneFront) {
      break;
    }
  }
  
  // 4. Calculate distances (duration * 0.01715)
  // If a sensor never finished (e.g., out of range), keep it at 999.0
  distLeft  = (doneLeft  && startLeft  > 0) ? (endLeft  - startLeft)  * 0.01715 : 999.0;
  distRight = (doneRight && startRight > 0) ? (endRight - startRight) * 0.01715 : 999.0;
  distFront = (doneFront && startFront > 0) ? (endFront - startFront) * 0.01715 : 999.0;
  
  // 5. Update boolean flags based on config threshold
  isWallFront = (distFront < WALL_DETECT_CM);
  isWallLeft  = (distLeft  < WALL_DETECT_CM);
  isWallRight = (distRight < WALL_DETECT_CM);
}

bool wallFront() { return isWallFront; }
bool wallLeft()  { return isWallLeft; }
bool wallRight() { return isWallRight; }
