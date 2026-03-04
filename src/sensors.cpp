#include "sensors.h"
#include <Arduino.h>

void initSensors() {
  // TODO: Set trig pins as OUTPUT, echo pins as INPUT
}

static float readUltrasonic(int trigPin, int echoPin) {
  // TODO: 5-sample averaged ultrasonic reading
  return 999.0;
}

float readFrontDistance() {
  return readUltrasonic(US_FRONT_TRIG, US_FRONT_ECHO);
}

float readLeftDistance() {
  return readUltrasonic(US_LEFT_TRIG, US_LEFT_ECHO);
}

float readRightDistance() {
  // TODO: Assign right sensor pins first
  return 999.0;
}

bool wallFront() { return readFrontDistance() < WALL_DETECT_CM; }
bool wallLeft()  { return readLeftDistance()  < WALL_DETECT_CM; }
bool wallRight() { return readRightDistance() < WALL_DETECT_CM; }
