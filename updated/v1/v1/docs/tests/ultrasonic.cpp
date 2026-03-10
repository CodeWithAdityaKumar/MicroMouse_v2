// ════════════════════════════════════════════════════════════
// HC-SR04 Ultrasonic Distance Sensor Test
// Attach TRIGGER_PIN and ECHO_PIN to available digital pins.
// Note: A3, D0, D1 are currently free/unused in your pin map.
// ════════════════════════════════════════════════════════════

#include <Arduino.h>

#define TRIGGER_PIN D0  // Example unused pin
#define ECHO_PIN    D1  // Example unused pin

void setup() {
    Serial.begin(115200);   // Note: D0 and D1 are typical Serial pins, 
                            // please adjust if using hardware Serial!
    
    // Suggestion: if you must use D0/D1 for Serial, then wire Trigger/Echo
    // to analog pins treated as digital (e.g., A3) or repurpose an IR pin.
    
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    Serial.println(F("HC-SR04 Ultrasonic Sensor Test"));
    Serial.println(F("──────────────────────────────"));
}

void loop() {
    // Clear trigger pin
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);

    // Send 10 microsecond pulse
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);

    // Read pulse length
    long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout

    if (duration == 0) {
        Serial.println(F("No Echo! (Timeout)"));
    } else {
        // Sound travels at ~343 m/s (or 0.0343 cm/µs)
        // Distance = (time * speed) / 2 (round trip)
        float distance_cm = (duration / 2.0) * 0.0343;

        Serial.print(F("Distance: "));
        Serial.print(distance_cm);
        Serial.println(F(" cm"));
    }

    delay(200);
}
