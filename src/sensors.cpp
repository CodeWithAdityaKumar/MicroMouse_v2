#include "include/sensors.h"
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

// Helper block to read a single ultrasonic via the shared trigger with a timeout
static float readUltrasonic(int echoPin) {
  // Clear trigger
  digitalWrite(US_TRIG, LOW);
  delayMicroseconds(2);
  
  // Send 10us pulse
  digitalWrite(US_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG, LOW);
  
  // Timeout set to 30000us (30ms) to ensure echo is captured reliably
  long duration = pulseIn(echoPin, HIGH, 30000);
  
  if (duration == 0) {
    return 999.0; // Out of range or no echo received
  }
  
  // Calculate distance in cm
  // Speed of sound = 340 m/s = 0.034 cm/us (duration * 0.0343 / 2.0 = duration * 0.01715)
  return duration * 0.01715;
}

void readAllSensorsBlocking() {
  distFront = readUltrasonic(US_FRONT_ECHO);
  delay(15);
  distLeft = readUltrasonic(US_LEFT_ECHO);
  delay(15);
  distRight = readUltrasonic(US_RIGHT_ECHO);
  delay(15);
  
  isWallFront = (distFront < WALL_DETECT_CM);
  isWallLeft  = (distLeft  < WALL_DETECT_CM);
  isWallRight = (distRight < WALL_DETECT_CM);
}

void updateSensors() {
  static unsigned long lastPing = 0;
  static int sensorState = 0;
  
  // Non-blocking round-robin: ping one sensor every 15ms.
  // This prevents the 135ms stall during the motion PID loop!
  if (millis() - lastPing >= 15) {
    switch (sensorState) {
      case 0:
        distFront = readUltrasonic(US_FRONT_ECHO);
        isWallFront = (distFront < WALL_DETECT_CM);
        sensorState = 1;
        break;
      case 1:
        distLeft = readUltrasonic(US_LEFT_ECHO);
        isWallLeft  = (distLeft  < WALL_DETECT_CM);
        sensorState = 2;
        break;
      case 2:
        distRight = readUltrasonic(US_RIGHT_ECHO);
        isWallRight = (distRight < WALL_DETECT_CM);
        sensorState = 0;
        break;
    }
    lastPing = millis();
  }
}

bool wallFront() { return isWallFront; }
bool wallLeft()  { return isWallLeft; }
bool wallRight() { return isWallRight; }
