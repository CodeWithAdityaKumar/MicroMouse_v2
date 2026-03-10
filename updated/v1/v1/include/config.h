#ifndef CONFIG_H
#define CONFIG_H

// ─── Motor Driver (TB6612FNG) — per Pins.md ───
// Left motor uses PWMA/AIN pins
#define PWMA    5     // Left motor speed (PWM)
#define AIN1    12    // Left motor dir A
#define AIN2    11    // Left motor dir B

// Right motor uses PWMB/BIN pins
#define PWMB    6     // Right motor speed (PWM)
#define BIN1    10    // Right motor dir A
#define BIN2    9     // Right motor dir B

#define STBY    13    // Standby (HIGH = enabled)

// ─── Encoders ──────────────────────────────────
#define ENC_LEFT_A   2   // INT0
#define ENC_LEFT_B   7
#define ENC_RIGHT_A  3   // INT1
#define ENC_RIGHT_B  8

// ─── IR Sensors ────────────────────────────────
#define IR_LEFT   A0
#define IR_FRONT  A1
#define IR_RIGHT  A2

// ─── Button ────────────────────────────────────
#define BUTTON_PIN  4

// ─── IMU (MPU6050) ─────────────────────────────
#define MPU_ADDR           0x68
#define GYRO_SENSITIVITY   131.0    // LSB/°/s at ±250°/s
#define GYRO_AXIS_YAW      'X'     // X-axis = yaw (MPU mounted horizontally)
#define CALIB_SAMPLES      2000
#define CALIB_DELAY_US     2000

// ─── Maze ──────────────────────────────────────
#define MAZE_SIZE        16
#define GOAL_ROW_MIN     7
#define GOAL_ROW_MAX     8
#define GOAL_COL_MIN     7
#define GOAL_COL_MAX     8

// ─── Motion ────────────────────────────────────
#define BASE_SPEED       120
#define TURN_SPEED       90
#define MIN_SPEED        60
#define TICKS_PER_CELL   1146   // Calibrate!
#define TICKS_PER_REV    20

// ─── PID Gains ─────────────────────────────────
#define HEADING_KP       5.5f
#define HEADING_KI       0.0f
#define HEADING_KD       1.8f
#define HEADING_I_LIMIT  500.0f
#define HEADING_OUT_MAX  255.0f

#define WALL_KP          2.0f
#define WALL_KI          0.0f
#define WALL_KD          0.3f

// ─── Sensor Thresholds ─────────────────────────
#define WALL_THRESHOLD     12.0f
#define NO_WALL_THRESHOLD  15.0f
#define COLLISION_DIST     3.0f

// ─── Timing ────────────────────────────────────
#define LOOP_INTERVAL_US   5000    // 5 ms → 200 Hz
#define DEBOUNCE_MS        50
#define LONG_PRESS_MS      1000
#define TURN_SETTLE_DEG    1.0f
#define TURN_SETTLE_RATE   5.0f    // °/s

// ─── Wall Bits ─────────────────────────────────
#define WALL_NORTH  0x08
#define WALL_EAST   0x04
#define WALL_SOUTH  0x02
#define WALL_WEST   0x01

// ─── Direction ─────────────────────────────────
#define DIR_NORTH  0
#define DIR_EAST   1
#define DIR_SOUTH  2
#define DIR_WEST   3

// ─── Debug ─────────────────────────────────────
#define DEBUG_SERIAL           true
#define SERIAL_BAUD            115200
#define DEBUG_PRINT_INTERVAL   200    // every 200th loop → ~1 Hz

#endif // CONFIG_H
