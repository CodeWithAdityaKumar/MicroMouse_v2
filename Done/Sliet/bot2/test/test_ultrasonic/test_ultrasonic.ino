// Test: Ultrasonic distance reading (shared trigger)
// Stage 1 — Step 2.4

#include "config.h"
#include "Arduino.h"

float readUltrasonic(int echoPin) {
  // Fire shared trigger
  digitalWrite(US_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(US_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return 999.0;
  return duration * 0.01715; // cm
}

float readFiltered(int echoPin) {
  float sum = 0;
  int valid = 0;
  for (int i = 0; i < 5; i++) {
    float d = readUltrasonic(echoPin);
    if (d < 400.0) { sum += d; valid++; }
    delay(30); // wait for echoes to die down between readings
  }
  return valid > 0 ? sum / valid : 999.0;
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println(F("Ultrasonic Sensor Test (Shared Trigger)"));
  Serial.println(F("Trig: A2 (shared) | Front Echo: A3 | Left Echo: A0 | Right Echo: A1"));
  Serial.println();

  pinMode(US_TRIG, OUTPUT);
  pinMode(US_FRONT_ECHO, INPUT);
  pinMode(US_LEFT_ECHO, INPUT);
  pinMode(US_RIGHT_ECHO, INPUT);
}

void loop() {
  // Read sequentially with delays to avoid cross-talk
  float front = readFiltered(US_FRONT_ECHO);
  delay(30);
  float left  = readFiltered(US_LEFT_ECHO);
  delay(30);
  float right = readFiltered(US_RIGHT_ECHO);

  Serial.print(F("Front: ")); Serial.print(front, 1); Serial.print(F(" cm  "));
  Serial.print(F("Left: "));  Serial.print(left, 1);  Serial.print(F(" cm  "));
  Serial.print(F("Right: ")); Serial.print(right, 1); Serial.println(F(" cm"));

  delay(200);
}
