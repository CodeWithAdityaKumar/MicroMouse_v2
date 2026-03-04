// Test: Ultrasonic distance reading
// Stage 1 — Step 2.4

#include "../src/config.h"

float readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return 999.0;
  return duration * 0.01715; // cm
}

float readFiltered(int trigPin, int echoPin) {
  float sum = 0;
  int valid = 0;
  for (int i = 0; i < 5; i++) {
    float d = readUltrasonic(trigPin, echoPin);
    if (d < 400.0) { sum += d; valid++; }
    delayMicroseconds(200);
  }
  return valid > 0 ? sum / valid : 999.0;
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println(F("Ultrasonic Sensor Test"));

  pinMode(US_FRONT_TRIG, OUTPUT);
  pinMode(US_FRONT_ECHO, INPUT);
  pinMode(US_LEFT_TRIG, OUTPUT);
  pinMode(US_LEFT_ECHO, INPUT);
}

void loop() {
  float front = readFiltered(US_FRONT_TRIG, US_FRONT_ECHO);
  float left  = readFiltered(US_LEFT_TRIG,  US_LEFT_ECHO);

  Serial.print(F("Front: ")); Serial.print(front, 1); Serial.print(F(" cm  "));
  Serial.print(F("Left: "));  Serial.print(left, 1);  Serial.println(F(" cm"));

  delay(200);
}
