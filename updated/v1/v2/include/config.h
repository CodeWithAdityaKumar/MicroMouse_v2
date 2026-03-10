// ================================================================
//  Config.h — Micromouse Firmware Configuration
//
//  All pin definitions, tuning constants, and configuration macros
//  in a single header. No magic numbers in main.ino.
//
//  Pin mapping matches the TESTED wiring from mpu.ino & rotate90deg.ino.
//
//  IMPORTANT — TB6612FNG label vs. code label:
//    The #define names PWMA/AIN1/AIN2 refer to the LEFT motor in code.
//    The #define names PWMB/BIN1/BIN2 refer to the RIGHT motor in code.
//    This matches the physical wiring verified in mpu.ino and rotate90deg.ino.
// ================================================================

#ifndef CONFIG_H
#define CONFIG_H

// ======================== MOTOR DRIVER (TB6612FNG) ========================
//
//  ** Wiring matches mpu.ino / rotate90deg.ino exactly **
//
//  In setMotors(left, right):
//    "left"  drives PWMA/AIN1/AIN2 (D6/D10/D9)
//    "right" drives PWMB/BIN1/BIN2 (D5/D12/D11)
//

// Left motor (controlled by PWMA/AIN1/AIN2 in code)
#define PWMA    6       // PWM speed  — left motor
#define AIN1    10      // Direction A — left motor
#define AIN2    9       // Direction B — left motor

// Right motor (controlled by PWMB/BIN1/BIN2 in code)
#define PWMB    5       // PWM speed  — right motor
#define BIN1    12      // Direction A — right motor
#define BIN2    11      // Direction B — right motor

#define STBY    13      // Standby (HIGH = motors enabled)

// ======================== ENCODERS (Quadrature) ========================
#define ENC_LEFT_A      2       // INT0 — left wheel tick
#define ENC_LEFT_B      7       // Direction — left wheel
#define ENC_RIGHT_A     3       // INT1 — right wheel tick
#define ENC_RIGHT_B     8       // Direction — right wheel

// ======================== WALL SENSORS (Sharp IR) ========================
#define SENSOR_LEFT_PIN     A0
#define SENSOR_FRONT_PIN    A1
#define SENSOR_RIGHT_PIN    A2

// ======================== BUTTON ========================
#define BUTTON_PIN      4       // INPUT_PULLUP, active LOW

// ======================== BATTERY MONITOR ========================
#define BATT_PIN        A3      // Voltage divider input

// ======================== IMU (MPU6050 over I²C) ========================
// SDA = A4, SCL = A5 — handled by Wire.h, no define needed.

// ======================== SERIAL ========================
// D0 (RX), D1 (TX) — reserved for Serial debug at 115200 baud.
#define SERIAL_BAUD     115200


// ================================================================
//  TUNING CONSTANTS — adjust on-robot
// ================================================================

// ---------- Control Loop ----------
//  200 Hz = 5000 µs interval (non-blocking via micros())
#define LOOP_INTERVAL_US        5000UL

// ---------- Motor Speeds (0–255 PWM) ----------
#define BASE_SPEED              80
#define TURN_SPEED              90
#define MAX_SPEED               255
#define MIN_SPEED               0

// ---------- PID — Heading ----------
//  Gains from mpu.ino (tested and tuned):
#define KP_HEADING              5.5f
#define KI_HEADING              0.0f        // Start at 0; add if steady-state drift
#define KD_HEADING              1.8f
#define MAX_INTEGRAL_HEADING    500.0f      // Anti-windup clamp

// ---------- PD — Wall Centering ----------
#define KP_WALL                 0.6f
#define KD_WALL                 0.2f

// ---------- Wall-Alignment Drift Correction ----------
//  Nudge gyro angle when both walls visible
#define WALL_ALIGN_GAIN         0.03f

// ---------- Wall Sensor Thresholds (10-bit ADC, 0–1023) ----------
//  Higher value = closer wall (Sharp IR characteristic)
//  NOTE: Tune these by reading serial debug with a wall at known distance.
#define WALL_DETECT_THRESHOLD   250
#define WALL_CLOSE_THRESHOLD    400
#define WALL_TOO_CLOSE          600
#define WALL_FRONT_STOP         500         // Stop threshold for front wall

// ---------- Sensor Averaging ----------
#define SENSOR_SAMPLES          6

// ---------- Encoder — Cell Distance ----------
//  Number of encoder ticks equivalent to one maze cell (~18 cm).
//  Formula: ticks = (cell_mm / (π × wheel_diameter_mm)) × ticks_per_rev
//  *** MEASURE AND TUNE THIS ON YOUR ROBOT ***
#define ONE_CELL_TICKS          350L

// ---------- Encoder — Turn Distance ----------
//  Encoder tick count for a 90° pivot turn (used only as safety timeout;
//  primary turn measurement is via IMU).
#define TICKS_PER_90_SAFETY     120L

// ---------- Gyro ----------
#define GYRO_SENSITIVITY        131.0f      // LSB per °/s at ±250°/s scale
#define GYRO_CALIB_SAMPLES      2000        // Match mpu.ino's tested value
#define GYRO_CALIB_DELAY_US     2000        // 2 ms between samples (~4 s total)
#define GYRO_MAX_VARIANCE       1000.0f     // Match mpu.ino's tested threshold
#define GYRO_CALIB_ATTEMPTS     3

// ---------- Turn Parameters ----------
#define TURN_SETTLE_RATE        3.0f        // °/s — below this = settled
#define TURN_SETTLE_TIME_MS     150UL       // Must stay settled for this long
#define TURN_TIMEOUT_MS         3000UL      // Safety abort if turn takes too long

// ---------- Battery Monitor ----------
//  V_bat = ADC × (5.0 / 1023.0) × VBAT_SCALE
//  Set VBAT_SCALE = (R1 + R2) / R2 for your voltage divider.
#define VBAT_SCALE              2.0f        // Example: 10kΩ + 10kΩ divider
#define VBAT_MIN                6.0f        // Low-battery threshold (V)
#define BATT_READ_INTERVAL_MS   1000UL

// ---------- Button Debounce ----------
#define DEBOUNCE_MS             50UL

// ---------- Debug ----------
#define DEBUG_PRINT_INTERVAL    40          // Print every Nth loop (~5 Hz at 200 Hz)

// ---------- Sensor Calibration Display ----------
#define SENSOR_CAL_DURATION_MS  3000UL      // Show live readings for 3 sec on boot


#endif // CONFIG_H
