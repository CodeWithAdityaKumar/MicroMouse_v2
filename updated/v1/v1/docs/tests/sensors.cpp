// ════════════════════════════════════════════════════════════
//  Phase 1.3: IR Sensor Test — 3× Sharp GP2Y0A21YK0F
//  Reads left (A0), front (A1), right (A2) IR sensors.
//  Prints raw ADC + distance (cm) + wall classification.
//  Hold objects at known distances to calibrate.
// ════════════════════════════════════════════════════════════

#include <Arduino.h>
#include "config.h"

// ADC → distance conversion for Sharp GP2Y0A21YK0F (10–80 cm)
float adcToDistance(int adc) {
    if (adc < 10) return 80.0;  // out of range
    return 2076.0 / (adc - 11.0);  // calibrated for 5V
}

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(500);

    Serial.println(F("══════════════════════════════════════════"));
    Serial.println(F("  Phase 1.3: IR Sensor Test (3× Sharp)"));
    Serial.println(F("══════════════════════════════════════════"));
    Serial.println(F("Place objects at known distances to verify."));
    Serial.println(F("Wall threshold: < 12 cm = WALL"));
    Serial.println(F("──────────────────────────────────────────"));
    Serial.println(F("  LEFT(A0)       FRONT(A1)      RIGHT(A2)"));
    Serial.println(F(" ADC  cm  wall  ADC  cm  wall  ADC  cm  wall"));
}

void loop() {
    // Read 4 samples per sensor and average (noise rejection)
    long sumL = 0, sumF = 0, sumR = 0;
    for (int i = 0; i < 4; i++) {
        sumL += analogRead(IR_LEFT);
        sumF += analogRead(IR_FRONT);
        sumR += analogRead(IR_RIGHT);
        delay(2);
    }

    int adcL = sumL / 4;
    int adcF = sumF / 4;
    int adcR = sumR / 4;

    float distL = adcToDistance(adcL);
    float distF = adcToDistance(adcF);
    float distR = adcToDistance(adcR);

    bool wallL = (distL < WALL_THRESHOLD);
    bool wallF = (distF < WALL_THRESHOLD);
    bool wallR = (distR < WALL_THRESHOLD);

    // Print: ADC cm wall | ADC cm wall | ADC cm wall
    Serial.print(adcL); Serial.print("\t");
    Serial.print(distL, 1); Serial.print("\t");
    Serial.print(wallL ? "WALL" : "----"); Serial.print("\t| ");

    Serial.print(adcF); Serial.print("\t");
    Serial.print(distF, 1); Serial.print("\t");
    Serial.print(wallF ? "WALL" : "----"); Serial.print("\t| ");

    Serial.print(adcR); Serial.print("\t");
    Serial.print(distR, 1); Serial.print("\t");
    Serial.println(wallR ? "WALL" : "----");

    delay(500);  // Print ~2 lines per second
}