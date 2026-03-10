// Test: Motor forward/backward
// Stage 1 — Step 2.1

#include "config.h"
#include "Arduino.h"

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println(F("Motor Test"));

  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);

  digitalWrite(STBY, HIGH);
}

void loop() {
  // PWMA/AIN = Left motor, PWMB/BIN = Right motor
  Serial.println(F("Left Forward"));
  digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); analogWrite(PWMA, 100);
  delay(2000);

  Serial.println(F("Left Backward"));
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH); analogWrite(PWMA, 100);
  delay(2000);

  // Stop
  analogWrite(PWMA, 0);

  Serial.println(F("Right Forward"));
  digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); analogWrite(PWMB, 100);
  delay(2000);

  Serial.println(F("Right Backward"));
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH); analogWrite(PWMB, 100);
  delay(2000);

  // Stop all
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);

  Serial.println(F("Both Forward"));
  digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW); analogWrite(PWMA, 100);
  digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW); analogWrite(PWMB, 100);
  delay(2000);

  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);

  Serial.println(F("Done. Waiting 5s..."));
  delay(5000);
}
