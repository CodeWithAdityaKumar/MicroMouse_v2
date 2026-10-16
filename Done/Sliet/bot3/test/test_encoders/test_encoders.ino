// Test: Encoder tick counting
// Stage 1 — Step 2.2

#include "config.h"
#include "Arduino.h"

volatile long leftTicks = 0;
volatile long rightTicks = 0;

void leftISR() {
  if (digitalRead(ENC_L_B) == LOW) leftTicks++;
  else leftTicks--;
}

void rightISR() {
  if (digitalRead(ENC_R_B) == HIGH) rightTicks++;
  else rightTicks--;
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println(F("Encoder Test"));
  Serial.println(F("Rotate wheels by hand. 1 revolution ≈ 355 ticks."));

  pinMode(ENC_L_A, INPUT);
  pinMode(ENC_L_B, INPUT);
  pinMode(ENC_R_A, INPUT);
  pinMode(ENC_R_B, INPUT);

  attachInterrupt(digitalPinToInterrupt(ENC_L_A), leftISR, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_R_A), rightISR, RISING);
}

void loop() {
  noInterrupts();
  long l = leftTicks;
  long r = rightTicks;
  interrupts();

  Serial.print(F("L: ")); Serial.print(l);
  Serial.print(F("  R: ")); Serial.println(r);

  delay(200);
}
