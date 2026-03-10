// Motor Test: forward → reverse → stop (both motors)
#include <Arduino.h>
#include "config.h"

void setup() {
    Serial.begin(115200);

    pinMode(STBY, OUTPUT);
    pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT); pinMode(PWMA, OUTPUT);
    pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT); pinMode(PWMB, OUTPUT);

    digitalWrite(STBY, HIGH);  // enable motor driver

    // ── LEFT MOTOR FORWARD ──
    Serial.println("Left motor: FORWARD");
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, 150);
    delay(2000);

    // ── LEFT MOTOR REVERSE ──
    Serial.println("Left motor: REVERSE");
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    analogWrite(PWMA, 150);
    delay(2000);

    // ── LEFT MOTOR STOP ──
    analogWrite(PWMA, 0);
    Serial.println("Left motor: STOP");
    delay(1000);

    // ── RIGHT MOTOR FORWARD ──
    Serial.println("Right motor: FORWARD");
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, 150);
    delay(2000);

    // ── RIGHT MOTOR REVERSE ──
    Serial.println("Right motor: REVERSE");
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    analogWrite(PWMB, 150);
    delay(2000);

    // ── RIGHT MOTOR STOP ──
    analogWrite(PWMB, 0);
    Serial.println("Right motor: STOP");
    delay(1000);

    // ── BOTH MOTORS FORWARD ──
    Serial.println("Both motors: FORWARD");
    digitalWrite(AIN1, HIGH); digitalWrite(AIN2, LOW);
    digitalWrite(BIN1, HIGH); digitalWrite(BIN2, LOW);
    analogWrite(PWMA, 150);
    analogWrite(PWMB, 150);
    delay(2000);

    // ── BOTH MOTORS REVERSE ──
    Serial.println("Both motors: REVERSE");
    digitalWrite(AIN1, LOW); digitalWrite(AIN2, HIGH);
    digitalWrite(BIN1, LOW); digitalWrite(BIN2, HIGH);
    analogWrite(PWMA, 150);
    analogWrite(PWMB, 150);
    delay(2000);

    // ── ALL STOP ──
    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
    digitalWrite(STBY, LOW);
    Serial.println("ALL STOP — test complete");
}

void loop() {}