#include "encoders.h"
#include <Arduino.h>

volatile long leftTicks = 0;
volatile long rightTicks = 0;

void leftISR() {
  // Left motor forward = +ve ticks
  if (digitalRead(ENC_L_B) == HIGH)
    leftTicks++;
  else
    leftTicks--;
}

void rightISR() {
  // Right motor forward = -ve ticks
  if (digitalRead(ENC_R_B) == HIGH)
    rightTicks++;
  else
    rightTicks--;
}

void initEncoders() {
  pinMode(ENC_L_A, INPUT);
  pinMode(ENC_L_B, INPUT);
  pinMode(ENC_R_A, INPUT);
  pinMode(ENC_R_B, INPUT);

  // Left encoder on D3 (INT1), Right encoder on D2 (INT0)
  attachInterrupt(digitalPinToInterrupt(ENC_L_A), leftISR, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_R_A), rightISR, RISING);
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
