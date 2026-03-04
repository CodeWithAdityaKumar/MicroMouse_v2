#pragma once

// ─── Pin Definitions ───────────────────────────
// Motor Driver (TB6612FNG)
#define PWMA     5    // Right motor PWM
#define AIN1     12   // Right motor dir
#define AIN2     11   // Right motor dir
#define PWMB     6    // Left motor PWM
#define BIN1     10   // Left motor dir
#define BIN2     9    // Left motor dir
#define STBY     8    // Standby (HIGH = active)

// Encoders
#define ENC_L_A  2    // Left encoder channel A (INT0)
#define ENC_L_B  4    // Left encoder channel B
#define ENC_R_A  3    // Right encoder channel A (INT1)
#define ENC_R_B  7    // Right encoder channel B

// Ultrasonic Sensors
#define US_FRONT_TRIG  A0
#define US_FRONT_ECHO  A1
#define US_LEFT_TRIG   A2
#define US_LEFT_ECHO   A3
// Right sensor: A6/A7 are analog-only — reassign to digital pins
// #define US_RIGHT_TRIG  ??
// #define US_RIGHT_ECHO  ??

// Battery
#define BATTERY_PIN    A6
#define V_DIVIDER_RATIO 4.0

// ─── Robot Physical Constants ──────────────────
#define WHEEL_DIAMETER_MM    34.0
#define WHEEL_CIRC_MM        106.81  // π × 34
#define ENCODER_CPR          355
#define MM_PER_TICK          0.3009  // 106.81 / 355
#define CELL_SIZE_MM         160
#define TICKS_PER_CELL       532     // 160 / 0.3009

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
#define MAX_INTEGRAL   1000.0
#define BASE_SPEED     100
#define SPEED_RUN_SPEED 150
#define MAX_TURN_SPEED 150
#define MIN_TURN_SPEED 40

// ─── Sensor Thresholds ─────────────────────────
#define WALL_DETECT_CM   10.0
#define WALL_CLEAR_CM    14.0
#define WALL_DANGER_CM   5.0
#define WALL_CENTER_CM   7.5

// ─── Timing ────────────────────────────────────
#define LOOP_INTERVAL_US   5000   // 200 Hz
#define TURN_SETTLE_MS     200
#define ANGLE_TOLERANCE    2.0    // degrees

// ─── Serial ────────────────────────────────────
#define SERIAL_BAUD        115200

// ─── Battery Thresholds ────────────────────────
#define BATTERY_LOW_V      10.5
#define BATTERY_CRITICAL_V 9.9

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
