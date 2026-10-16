// Test: Stage 3 Wall Detection
// Reads the 3 ultrasonic sensors using the shared trigger pin and 
// displays the distance (cm) and the boolean threshold logic.

#include "config.h"
#include "sensors.h"
#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.println(F("=== Stage 3: Ultrasonic Wall Detection Test ==="));
  Serial.println(F("Initializing sensors..."));
  
  initSensors();
  
  Serial.println(F("Sensors ready."));
  Serial.print(F("Wall detection threshold: "));
  Serial.print(WALL_DETECT_CM);
  Serial.println(F(" cm"));
  Serial.println(F("--------------------------------------------------"));
}

void loop() {
  // 1. Poll the shared trigger and read all 3 echoes sequentially
  updateSensors();

  // 2. Format the output string for easy reading
  // Format: [LEFT: 4.5cm WALL] -- [FRONT: 15.2cm CLEAR] -- [RIGHT: out of range]
  
  Serial.print(F("[LEFT: "));
  if (distLeft > 50.0) {
    Serial.print(F("OOR      ]")); // Out Of Range
  } else {
    Serial.print(distLeft, 1);
    Serial.print(F("cm "));
    Serial.print(wallLeft() ? F("WALL] ") : F("CLR ] "));
  }

  Serial.print(F("  --  [FRONT: "));
  if (distFront > 50.0) {
    Serial.print(F("OOR      ]"));
  } else {
    Serial.print(distFront, 1);
    Serial.print(F("cm "));
    Serial.print(wallFront() ? F("WALL] ") : F("CLR ] "));
  }

  Serial.print(F("  --  [RIGHT: "));
  if (distRight > 50.0) {
    Serial.print(F("OOR      ]"));
  } else {
    Serial.print(distRight, 1);
    Serial.print(F("cm "));
    Serial.print(wallRight() ? F("WALL] ") : F("CLR ] "));
  }

  Serial.println();
  
  // Wait before next reading so Serial Monitor is readable
  delay(150);
}
