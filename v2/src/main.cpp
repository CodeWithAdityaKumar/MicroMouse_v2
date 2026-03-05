// Test: Stage 2 Motion Control
// Tests moving 1 cell forward, turning exactly 90 degrees right, 90 left, and 180 degrees back.

#include "Arduino.h"
#include "config.h"
#include "motors.h"
#include "encoders.h"
#include "imu.h"
#include "motion.h"
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Serial.println(F("=== Stage 2 Motion Test ==="));

  // Initialize all subsystems
  initMotion();
  
  if (!initIMU()) {
    Serial.println(F("MPU6050 init failed!"));
    while (1);
  }

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  Serial.println(F("Calibrating IMU... Keep still for 3s."));
  delay(500);
  
  if (!calibrateGyro()) {
    Serial.println(F("IMU calibration failed - too much movement."));
    while(1);
  }

  Serial.println(F("IMU Calibrated."));
  Serial.println(F("Press the push button (D4) to start the test."));

  // Wait for button press
  while (digitalRead(BUTTON_PIN) == HIGH) {
    delay(10);
  }
  delay(500); // Debounce / let go

  Serial.println(F("Starting sequence..."));

  // 1. Move 1 cell
  Serial.println(F("Moving forward 1 cell (16cm)..."));
  moveForward(16.0);
  delay(1000);

  // 2. Turn 90 Right
  Serial.println(F("Turning 90 degrees Right..."));
  turnRight90();
  delay(1000);

  // 3. Turn 90 Left
  Serial.println(F("Turning 90 degrees Left (returning to straight)..."));
  turnLeft90();
  delay(1000);

  // 4. Turn 180 degrees Back
  Serial.println(F("Turning 180 degrees Back..."));
  turnBack();
  delay(1000);



  Serial.println(F("Test Complete!"));
}

void loop() {
  // Nothing to do after test sequence
  delay(100);
}
