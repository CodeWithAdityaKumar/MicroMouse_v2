#pragma once

// ─── Pin Definitions ───────────────────────────
// Motor Driver (TB6612FNG)
#define PWMA     6    // Left motor PWM (previously 5)
#define AIN1     9    // Left motor dir (swapped to fix left rotation)
#define AIN2     10   // Left motor dir (swapped to fix left rotation)
#define PWMB     5    // Right motor PWM (previously 6)
#define BIN1     12   // Right motor dir (previously 10)
#define BIN2     11   // Right motor dir (previously 9)
#define STBY     13   // Standby (HIGH = active)

// Encoders
#define ENC_L_A  2    // Left encoder channel A (INT0)
#define ENC_L_B  7    // Left encoder channel B
#define ENC_R_A  3    // Right encoder channel A (INT1)
#define ENC_R_B  8    // Right encoder channel B

// Ultrasonic Sensors (shared trigger — all 3 trigs wired to A2)
#define US_TRIG        A2   // Shared trigger pin for all sensors
#define US_FRONT_ECHO  A3   // Front echo
#define US_LEFT_ECHO   A0   // Left echo
#define US_RIGHT_ECHO  A1   // Right echo

// Power Regulation
// L7805CV linear voltage regulator: Battery → 5V (not connected to any pin)

// Push Button (start/stop)
#define BUTTON_PIN     4    // D4 — uses INPUT_PULLUP (press = LOW)
#define DEBOUNCE_MS    50   // Debounce delay

// ─── IMU (MPU6050) ─────────────────────────────
#define MPU_ADDR       0x68
// #define GYRO_REG       0x43  // GYRO_XOUT_H (X-axis for yaw)
#define GYRO_REG       0x47  // GYRO_XOUT_H (Z-axis for yaw)
#define GYRO_SENS      131.0 // ±250°/s sensitivity
// MPU mounted on side — X-axis = yaw
// Left turn = +ve angle, Right turn = -ve angle

// ─── Robot Physical Constants ──────────────────
#define WHEEL_DIAMETER_MM    40.0
#define WHEEL_CIRC_MM        125.71  // π × 34
#define ENCODER_CPR          355
#define MM_PER_TICK          0.3541  // 106.81 / 355
#define CELL_SIZE_MM         160
#define TICKS_PER_CELL       452     // 160 / 0.3009

// #define WHEEL_DIAMETER_MM    34.0
// #define WHEEL_CIRC_MM        106.81  // π × 34
// #define ENCODER_CPR          355
// #define MM_PER_TICK          0.3009  // 106.81 / 355
// #define CELL_SIZE_MM         160
// #define TICKS_PER_CELL       532     // 160 / 0.3009

// Tunable Turn Ticks (Adjust these based on physical testing!)
#define TICKS_PER_90DEG_L    300/2     // Ticks for Left 90° turn
#define TICKS_PER_90DEG_R    300/2     // Ticks for Right 90° turn
#define TICKS_PER_180DEG     320       // Ticks for 180° turn (tune this separately!)

// ─── Maze Constants ────────────────────────────
#define MAZE_SIZE            8
#define GOAL_X1 3
#define GOAL_Y1 3
#define GOAL_X2 4
#define GOAL_Y2 4

// ─── PID Constants ─────────────────────────────
#define STRAIGHT_KP    2.0
#define STRAIGHT_KI    0.0
#define STRAIGHT_KD    1.0
#define TURN_KP        4.0
#define WALL_KP        0.5
#define TURN_P         2.5    // Loop multiplier for Proportional turns
#define TURN_MIN_SPEED 45     // Minimum speed to prevent motor stall
#define MAX_INTEGRAL   1000.0
#define BASE_SPEED     80
#define SPEED_RUN_SPEED 150
#define MAX_TURN_SPEED 150
#define MIN_TURN_SPEED 40

// ─── Sensor Thresholds ─────────────────────────
#define WALL_DETECT_CM   10.0   // Wall-centering PID threshold
#define MAZE_WALL_CM     7.0    // Maze recording threshold (more conservative)
                                // Actual adjacent wall is ~3.5cm. Far perpendicular
                                // corridor walls read ~8-9cm - must NOT be flagged.
#define WALL_CLEAR_CM    14.0
#define WALL_DANGER_CM   2.0    // Too close to crash
#define WALL_CENTER_CM   3.5    // (16cm cell - 9cm bot) / 2 = 3.5cm ideal gap

// ─── Timing ────────────────────────────────────
#define LOOP_INTERVAL_US   5000   // 200 Hz
#define TURN_SETTLE_MS     200
#define ANGLE_TOLERANCE    2.0    // degrees

// ─── Serial ────────────────────────────────────
#define SERIAL_BAUD        115200

// ─── Battery Thresholds ────────────────────────
// No battery monitoring — using L7805CV regulator

// ─── Direction Enum ────────────────────────────
enum Direction { NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3 };

// ─── Wall Bitmasks ─────────────────────────────
#define WALL_N  0x01
#define WALL_E  0x02
#define WALL_S  0x04
#define WALL_W  0x08

// ─── Robot States ──────────────────────────────
enum BotState {
  STATE_IDLE,
  STATE_CALIBRATING,
  STATE_EXPLORING,
  STATE_TURNING,
  STATE_MOVING,
  STATE_SPEED_RUN,
  STATE_STOPPED
};
