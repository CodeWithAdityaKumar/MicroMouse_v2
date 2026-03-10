# Micromouse v1 — MVP Technical Documentation

> **Version:** 1.0  
> **Date:** 2026-03-02  
> **Platform:** Arduino Nano (ATmega328P) · PlatformIO  
> **Scope:** Minimum Viable Product — what to build first

---

## 1. MVP Definition

The MVP micromouse must achieve **one complete maze solve** — explore a 16×16 maze, reach the centre goal, and return to start. No speed run optimisation, no diagonal moves, no EEPROM persistence.

### MVP = Explore → Reach Goal → Return to Start

---

## 2. MVP Feature Set

| Feature                              | Included | Deferred |
| ------------------------------------ | -------- | -------- |
| Gyro-based heading PID               | ✅       | —        |
| Wall sensing (3× IR)                 | ✅       | —        |
| Encoder odometry (cell counting)     | ✅       | —        |
| 90° and 180° pivot turns             | ✅       | —        |
| Flood fill exploration               | ✅       | —        |
| Return to start (reverse flood fill) | ✅       | —        |
| Speed run                            | ❌       | Phase 2  |
| Smooth curves / diagonals            | ❌       | Phase 3  |
| EEPROM map storage                   | ❌       | Phase 2  |
| Battery voltage monitoring           | ❌       | Phase 2  |
| LED/buzzer feedback                  | ❌       | Phase 2  |

---

## 3. MVP Build Phases

### Phase 1: Hardware Verification (1–2 days)

**Goal:** Confirm every sensor / actuator works independently.

#### 1.1 Motor Test

```cpp
// Test: both motors spin forward for 2 seconds, then stop
#include <Arduino.h>
#include "config.h"

void setup() {
    pinMode(STBY, OUTPUT);
    pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);

    digitalWrite(STBY, HIGH);  // enable

    // Left forward
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, 150);

    // Right forward
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, 150);

    delay(2000);

    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
    digitalWrite(STBY, LOW);
}

void loop() {}
```

#### 1.2 IMU Test

```cpp
#include <Wire.h>

void setup() {
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000);

    // Wake up MPU6050
    Wire.beginTransmission(0x68);
    Wire.write(0x6B);  // PWR_MGMT_1
    Wire.write(0x00);  // Wake up
    Wire.endTransmission();

    // Set gyro range ±250°/s
    Wire.beginTransmission(0x68);
    Wire.write(0x1B);
    Wire.write(0x00);
    Wire.endTransmission();
}

void loop() {
    Wire.beginTransmission(0x68);
    Wire.write(0x47);  // GYRO_ZOUT_H
    Wire.endTransmission(false);
    Wire.requestFrom(0x68, 2);
    int16_t gz = Wire.read() << 8 | Wire.read();

    float gyroZ = gz / 131.0;  // °/s at ±250 range
    Serial.println(gyroZ);
    delay(10);
}
```

#### 1.3 IR Sensor Test

```cpp
void setup() {
    Serial.begin(115200);
}

void loop() {
    int left  = analogRead(A0);
    int front = analogRead(A1);
    int right = analogRead(A2);

    Serial.print("L:"); Serial.print(left);
    Serial.print(" F:"); Serial.print(front);
    Serial.print(" R:"); Serial.println(right);
    delay(100);
}
```

#### 1.4 Encoder Test

```cpp
volatile long leftTicks = 0;
volatile long rightTicks = 0;

void leftISR()  { leftTicks += digitalRead(7) ? 1 : -1; }
void rightISR() { rightTicks += digitalRead(8) ? 1 : -1; }

void setup() {
    Serial.begin(115200);
    pinMode(2, INPUT);
    pinMode(3, INPUT);
    pinMode(7, INPUT);
    pinMode(8, INPUT);
    attachInterrupt(digitalPinToInterrupt(2), leftISR, RISING);
    attachInterrupt(digitalPinToInterrupt(3), rightISR, RISING);
}

void loop() {
    Serial.print("L:"); Serial.print(leftTicks);
    Serial.print(" R:"); Serial.println(rightTicks);
    delay(200);
}
```

---

### Phase 2: Drive Straight with PID (2–3 days)

**Goal:** Robot drives a straight line using gyro PID and encoder cell counting.

**Key parameters to calibrate:**

| Parameter              | How to Calibrate                                                            |
| ---------------------- | --------------------------------------------------------------------------- |
| Gyro bias              | Average 500 samples at rest                                                 |
| PID gains (Kp, Ki, Kd) | Start with Kp=3.0, Ki=0, Kd=0.5. Increase Kp until oscillation, then add Kd |
| Ticks per cell         | Drive 18 cm, record encoder ticks. Repeat 5× and average                    |
| Base speed (PWM)       | Start at 100, increase to 150 for reliability                               |

**Verification Criteria:**

- Drives 5 cells (90 cm) with < 2° heading deviation
- Stops at correct cell boundary (±5 mm)

---

### Phase 3: Turn Execution (1–2 days)

**Goal:** Reliable 90° and 180° pivot turns.

**Turn algorithm:**

```
function turn90(direction):
    targetYaw = currentYaw + (direction == RIGHT ? -90 : +90)

    while |currentYaw - targetYaw| > 1.0°:
        updateYaw()
        error = targetYaw - currentYaw
        turnPWM = constrain(Kp_turn * error, -200, 200)
        setMotors(-turnPWM, +turnPWM)  // pivot

    stopMotors()
    wait until angularRate < 5°/s (settle)
    lockHeading(targetYaw)
```

**Verification Criteria:**

- Turn error < ±3° over 20 consecutive turns
- Settle time < 500 ms

---

### Phase 4: Wall Sensing & Classification (1–2 days)

**Goal:** Reliable wall detection on all 3 sides.

**Calibration procedure:**

1. Place robot in known cell with walls on all 3 sides
2. Record raw ADC values → note readings for "wall present" (~4–8 cm)
3. Remove walls one at a time → note readings for "no wall" (~20+ cm)
4. Set threshold with hysteresis band

**Wall reading function:**

```cpp
struct WallReadings {
    float leftCm, frontCm, rightCm;
    bool leftWall, frontWall, rightWall;
};

WallReadings readWalls() {
    WallReadings w;

    // Average 4 samples
    int lSum = 0, fSum = 0, rSum = 0;
    for (int i = 0; i < 4; i++) {
        lSum += analogRead(A0);
        fSum += analogRead(A1);
        rSum += analogRead(A2);
    }

    w.leftCm  = adcToDistance(lSum / 4);
    w.frontCm = adcToDistance(fSum / 4);
    w.rightCm = adcToDistance(rSum / 4);

    w.leftWall  = (w.leftCm  < WALL_THRESHOLD);
    w.frontWall = (w.frontCm < WALL_THRESHOLD);
    w.rightWall = (w.rightCm < WALL_THRESHOLD);

    return w;
}
```

---

### Phase 5: Maze Mapping + Flood Fill (3–5 days)

**Goal:** Full exploration of a test maze using flood fill.

#### 5.1 Maze Data Structure

```cpp
// config.h
#define MAZE_SIZE 16
#define NORTH 0x08
#define EAST  0x04
#define SOUTH 0x02
#define WEST  0x01

// maze.h
struct Cell {
    uint8_t walls;      // 4-bit wall flags
    uint8_t floodValue; // 0–255 BFS distance
};

extern Cell maze[MAZE_SIZE][MAZE_SIZE];
extern bool visited[MAZE_SIZE][MAZE_SIZE];
```

#### 5.2 Flood Fill Implementation

```cpp
// navigation.cpp
#include "maze.h"

#define QUEUE_SIZE 256
uint8_t queue[QUEUE_SIZE];
uint8_t qHead = 0, qTail = 0;

void enqueue(uint8_t idx) { queue[qTail++] = idx; }
uint8_t dequeue()         { return queue[qHead++]; }
bool queueEmpty()         { return qHead == qTail; }

void floodFill() {
    // Reset all flood values
    for (int r = 0; r < 16; r++)
        for (int c = 0; c < 16; c++)
            maze[r][c].floodValue = 255;

    // Seed goal cells
    qHead = qTail = 0;
    maze[7][7].floodValue = 0; enqueue(7 * 16 + 7);
    maze[7][8].floodValue = 0; enqueue(7 * 16 + 8);
    maze[8][7].floodValue = 0; enqueue(8 * 16 + 7);
    maze[8][8].floodValue = 0; enqueue(8 * 16 + 8);

    // BFS expansion
    while (!queueEmpty()) {
        uint8_t idx = dequeue();
        uint8_t r = idx / 16;
        uint8_t c = idx % 16;
        uint8_t newVal = maze[r][c].floodValue + 1;

        // Check NORTH
        if (r < 15 && !(maze[r][c].walls & NORTH)) {
            if (maze[r+1][c].floodValue > newVal) {
                maze[r+1][c].floodValue = newVal;
                enqueue((r+1) * 16 + c);
            }
        }
        // Check EAST
        if (c < 15 && !(maze[r][c].walls & EAST)) {
            if (maze[r][c+1].floodValue > newVal) {
                maze[r][c+1].floodValue = newVal;
                enqueue(r * 16 + (c+1));
            }
        }
        // Check SOUTH
        if (r > 0 && !(maze[r][c].walls & SOUTH)) {
            if (maze[r-1][c].floodValue > newVal) {
                maze[r-1][c].floodValue = newVal;
                enqueue((r-1) * 16 + c);
            }
        }
        // Check WEST
        if (c > 0 && !(maze[r][c].walls & WEST)) {
            if (maze[r][c-1].floodValue > newVal) {
                maze[r][c-1].floodValue = newVal;
                enqueue(r * 16 + (c-1));
            }
        }
    }
}
```

#### 5.3 Decision Function

```cpp
uint8_t getNextDirection(uint8_t row, uint8_t col, uint8_t facing) {
    uint8_t bestDir = facing;  // prefer going straight
    uint8_t bestVal = 255;

    struct { int8_t dr, dc; uint8_t wallBit; uint8_t dir; } dirs[4] = {
        { 1,  0, NORTH, 0},  // NORTH
        { 0,  1, EAST,  1},  // EAST
        {-1,  0, SOUTH, 2},  // SOUTH
        { 0, -1, WEST,  3},  // WEST
    };

    for (int i = 0; i < 4; i++) {
        if (maze[row][col].walls & dirs[i].wallBit) continue;

        uint8_t nr = row + dirs[i].dr;
        uint8_t nc = col + dirs[i].dc;

        if (nr >= 16 || nc >= 16) continue;

        if (maze[nr][nc].floodValue < bestVal) {
            bestVal = maze[nr][nc].floodValue;
            bestDir = dirs[i].dir;
        }
    }

    return bestDir;
}
```

---

### Phase 6: Integration & Testing (2–3 days)

**Goal:** All subsystems working together in the main loop.

**Integration checklist:**

- [ ] Button triggers calibration → heading lock → exploration
- [ ] Robot drives into cell, stops at boundary
- [ ] Walls read and stored correctly
- [ ] Flood fill recalculated after wall update
- [ ] Robot turns to face lowest flood value neighbour
- [ ] Robot reaches goal cells
- [ ] Robot returns to (0,0) via reverse flood fill
- [ ] Long-press button stops robot from any state

---

## 4. Critical Calibration Parameters

| Parameter             | Variable          | How to Determine                          |
| --------------------- | ----------------- | ----------------------------------------- |
| Gyro Z bias           | `gyroBias`        | Average 500 stationary readings           |
| IR wall threshold     | `WALL_THRESHOLD`  | Measure ADC at known distances            |
| Ticks per cell        | `TICKS_PER_CELL`  | Drive 18 cm, count encoder ticks × 5      |
| Heading PID Kp        | `HEADING_KP`      | Increase until oscillation, then ×0.6     |
| Heading PID Kd        | `HEADING_KD`      | Add until oscillation damps in 1–2 cycles |
| Turn settle threshold | `TURN_SETTLE_DEG` | 0.5°–2.0° based on gyro noise floor       |
| Base motor speed      | `BASE_SPEED`      | 100–180 PWM (depends on battery voltage)  |
| Turn motor speed      | `TURN_SPEED`      | 80–120 PWM (slower = more accurate)       |

---

## 5. Config Header (config.h)

```cpp
#ifndef CONFIG_H
#define CONFIG_H

// ─── Pin Definitions ───────────────────────────
// Motor Driver (TB6612FNG)
#define PWMA    5     // Left motor speed (PWM)
#define AIN1    12    // Left motor dir A
#define AIN2    11    // Left motor dir B
#define PWMB    6     // Right motor speed (PWM)
#define BIN1    10    // Right motor dir A
#define BIN2    9     // Right motor dir B
#define STBY    13    // Standby (HIGH = enabled)

// Encoders
#define ENC_LEFT_A   2   // INT0
#define ENC_LEFT_B   7
#define ENC_RIGHT_A  3   // INT1
#define ENC_RIGHT_B  8

// IR Sensors
#define IR_LEFT   A0
#define IR_FRONT  A1
#define IR_RIGHT  A2

// Button
#define BUTTON_PIN  4

// ─── Maze ──────────────────────────────────────
#define MAZE_SIZE        16
#define GOAL_ROW_MIN     7
#define GOAL_ROW_MAX     8
#define GOAL_COL_MIN     7
#define GOAL_COL_MAX     8

// ─── Motion ────────────────────────────────────
#define BASE_SPEED       120    // PWM (0–255)
#define TURN_SPEED       90     // PWM during pivot turns
#define MIN_SPEED        60     // Below this, motors stall
#define TICKS_PER_CELL   1146   // Calibrate for your wheels

// ─── PID Gains ─────────────────────────────────
#define HEADING_KP       3.0f
#define HEADING_KI       0.01f
#define HEADING_KD       0.5f
#define HEADING_I_LIMIT  50.0f
#define HEADING_OUT_MAX  60.0f

#define WALL_KP          2.0f
#define WALL_KI          0.0f
#define WALL_KD          0.3f

// ─── Sensor Thresholds ─────────────────────────
#define WALL_THRESHOLD     12.0f   // cm — wall present
#define NO_WALL_THRESHOLD  15.0f   // cm — no wall
#define COLLISION_DIST     3.0f    // cm — emergency stop

// ─── Timing ────────────────────────────────────
#define LOOP_INTERVAL_US   5000   // 5 ms → 200 Hz
#define DEBOUNCE_MS        50
#define LONG_PRESS_MS      1000
#define TURN_SETTLE_DEG    1.0f
#define TURN_SETTLE_RATE   5.0f   // °/s

// ─── Wall Bits ─────────────────────────────────
#define WALL_NORTH  0x08
#define WALL_EAST   0x04
#define WALL_SOUTH  0x02
#define WALL_WEST   0x01

// ─── Direction Encoding ────────────────────────
#define DIR_NORTH  0
#define DIR_EAST   1
#define DIR_SOUTH  2
#define DIR_WEST   3

// ─── Debug ─────────────────────────────────────
#define DEBUG_SERIAL     true
#define SERIAL_BAUD      115200

#endif // CONFIG_H
```

---

## 6. SRAM Memory Map

```
Total SRAM: 2048 bytes

┌─────────────────────────────┬───────┐
│ Component                   │ Bytes │
├─────────────────────────────┼───────┤
│ maze[16][16] (Cell: 2B ea.) │  512  │
│ visited[16][16] (bool)      │  256  │
│ BFS queue[256]              │  256  │
│ PID instances (×2)          │   48  │
│ Yaw, dt, sensor floats      │   32  │
│ State machine vars          │   16  │
│ Encoder tick counters       │    8  │
│ Serial TX buffer            │   64  │
│ Stack & locals              │ ~700  │
├─────────────────────────────┼───────┤
│ TOTAL                       │~1892  │
│ FREE                        │ ~156  │
└─────────────────────────────┴───────┘
```

> **Optimisation:** Merge `visited` into the `Cell.walls` byte (use bit 4). Saves 256 bytes, bringing free SRAM to ~412 bytes.

---

## 7. Testing Strategy

### 7.1 Unit Tests (Serial Monitor)

| Test       | Method                      | Pass Criteria                             |
| ---------- | --------------------------- | ----------------------------------------- |
| Motors     | Run each motor individually | Both spin correct direction               |
| IMU        | Print gyro Z raw values     | Reads ~0 at rest, responds to rotation    |
| IR sensors | Print ADC & cm              | Correct distance at 5, 10, 20 cm          |
| Encoders   | Print tick counts           | Increments/decrements with wheel rotation |
| Button     | Print press events          | Detects short and long press              |

### 7.2 Integration Tests (Physical Maze)

| Test              | Setup                        | Pass Criteria                    |
| ----------------- | ---------------------------- | -------------------------------- |
| Straight 5 cells  | Empty corridor, 5 cells long | Arrives at cell 5 ±5 mm          |
| 90° turn accuracy | Open space                   | 20 consecutive turns < ±3° error |
| Wall detection    | Known 3-wall cell            | All 3 walls correctly identified |
| 4-cell maze       | 2×2 maze, 1 dead end         | Solves and reaches goal          |
| 8×8 maze          | Half-size maze               | Complete solve within 3 minutes  |
| Full 16×16 maze   | Competition maze             | Reach goal + return to start     |

---

## 8. Common Failure Modes & Fixes

| Symptom                | Likely Cause                      | Fix                                         |
| ---------------------- | --------------------------------- | ------------------------------------------- |
| Robot veers right      | Gyro bias not calibrated          | Re-calibrate; ensure robot is stationary    |
| Robot oscillates       | PID Kp too high                   | Reduce Kp by 30%, increase Kd               |
| Misses cell boundary   | Ticks per cell wrong              | Re-measure with tape measure at 18 cm       |
| False wall readings    | IR sensor noise                   | Increase averaging to 8 samples             |
| Robot freezes mid-maze | SRAM overflow / stack collision   | Reduce memory usage, check free RAM         |
| Turn overshoots        | Angular momentum + slow gyro read | Lower turn PWM, tighten settle check        |
| `does not name a type` | Arduino IDE prototype bug         | Move structs to `.h` files (use PlatformIO) |
| Motor doesn't start    | STBY pin LOW / PWM below min      | Check `digitalWrite(STBY, HIGH)`, PWM ≥ 60  |

---

## 9. MVP Timeline Summary

| Week       | Phase                     | Deliverable                                     |
| ---------- | ------------------------- | ----------------------------------------------- |
| **Week 1** | Hardware verification     | All sensors/actuators tested individually       |
| **Week 2** | Drive straight + turns    | PID-controlled straight driving, reliable turns |
| **Week 3** | Wall sensing + mapping    | Calibrated IR, maze data structure working      |
| **Week 4** | Flood fill + exploration  | Robot solves small test maze                    |
| **Week 5** | Full integration + tuning | Robot solves 16×16 maze, returns to start       |
