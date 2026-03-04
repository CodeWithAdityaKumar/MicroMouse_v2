#include "encoders.h"
#include <Arduino.h>

volatile long leftTicks = 0;
volatile long rightTicks = 0;

void leftISR() {
  // TODO: Read channel B to determine direction
}

void rightISR() {
  // TODO: Read channel B to determine direction
}

void initEncoders() {
  // TODO: Set pin modes and attach interrupts
}

void resetEncoders() {
  noInterrupts();
  leftTicks = 0;
  rightTicks = 0;
  interrupts();
}

long getLeftTicks() {
  noInterrupts();
  long t = leftTicks;
  interrupts();
  return t;
}

long getRightTicks() {
  noInterrupts();
  long t = rightTicks;
  interrupts();
  return t;
}
