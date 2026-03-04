# 🐭 Micromouse v2 — Product Requirements Document

> **Version**: 2.0  
> **Date**: 2026-03-05  
> **Status**: Active Development  
> **Platform**: Arduino Nano (ATmega328P)  
> **Maze Size**: 8×8 (16 cm × 16 cm cells)

---

## 1. Project Overview

Micromouse v2 is an autonomous maze-solving robot that navigates an 8×8 grid maze, discovers the optimal path to the center using the **Flood Fill algorithm**, and executes a high-speed return run. The robot uses differential drive, ultrasonic sensors for wall detection, encoder motors for odometry, and an MPU6050 IMU for heading stabilization.

### 1.1 Objectives

| #   | Objective                  | Success Criteria                                    |
| --- | -------------------------- | --------------------------------------------------- |
| 1   | **Autonomous Exploration** | Robot explores full maze without human intervention |
| 2   | **Reach Center**           | Robot reaches goal cells (3,3), (3,4), (4,3), (4,4) |
| 3   | **Optimal Pathing**        | Flood Fill produces shortest path                   |
| 4   | **Speed Run**              | Robot replays shortest path at higher speed         |
| 5   | **Reliability**            | Consistent runs without crashes or wall collisions  |

### 1.2 Constraints

- **Arduino Nano**: 32 KB flash, 2 KB SRAM — memory-efficient data structures required
- **Ultrasonic sensors**: ~30 ms per reading — slower than IR, limits max exploration speed
- **3S LiPo (11.7V)**: Requires voltage regulation and battery monitoring
- **College project scope**: Prioritize reliability over competition-grade speed

---

## 2. Hardware Specification

### 2.1 Bill of Materials

| Component       | Model / Spec                            | Qty |
| --------------- | --------------------------------------- | --- |
| Microcontroller | Arduino Nano (ATmega328P, 5V)           | 1   |
| Motor Driver    | TB6612FNG Dual H-Bridge                 | 1   |
| Motors          | N20 Encoder DC Motors                   | 2   |
| IMU             | MPU6050 (Gyro + Accelerometer)          | 1   |
| Wall Sensors    | HC-SR04 Ultrasonic (×3)                 | 3   |
| Battery         | 3S LiPo 11.1V (~11.7V full)             | 1   |
| Voltage Divider | Resistor network for battery monitoring | 1   |
| Wheels          | 34 mm diameter                          | 2   |
| Caster          | Ball caster (rear support)              | 1   |

### 2.2 Pin Mapping

#### Motor Driver (TB6612FNG → Arduino Nano)

| Signal | Arduino Pin | Function                       |
| ------ | ----------- | ------------------------------ |
| PWMA   | D5          | Right motor speed              |
| AIN1   | D12         | Right motor direction          |
| AIN2   | D11         | Right motor direction          |
| PWMB   | D6          | Left motor speed               |
| BIN1   | D10         | Left motor direction           |
| BIN2   | D9          | Left motor direction           |
| STBY   | D8          | Driver standby (HIGH = active) |

#### Encoders

| Signal          | Arduino Pin | Notes                   |
| --------------- | ----------- | ----------------------- |
| Left Encoder A  | D2          | Hardware interrupt INT0 |
| Left Encoder B  | D4          | Direction detection     |
| Right Encoder A | D3          | Hardware interrupt INT1 |
| Right Encoder B | D7          | Direction detection     |

#### MPU6050 (I²C)

| Signal | Arduino Pin |
| ------ | ----------- |
| VCC    | 5V          |
| GND    | GND         |
| SDA    | A4          |
| SCL    | A5          |

#### Ultrasonic Sensors

| Sensor | Trig Pin | Echo Pin |
| ------ | -------- | -------- |
| Front  | A0       | A1       |
| Left   | A2       | A3       |
| Right  | A6       | A7       |

> ⚠️ **Note**: A6/A7 on Nano are **analog-only** pins — they cannot do `digitalRead()`. Echo signals on these pins require `analogRead()` threshold detection or use a different pin assignment.

#### Battery Monitor

| Signal                 | Arduino Pin |
| ---------------------- | ----------- |
| Voltage Divider Output | A6 or A7    |

### 2.3 Key Calculations

| Parameter                 | Value             | Derivation             |
| ------------------------- | ----------------- | ---------------------- |
| Wheel circumference       | **106.8 mm**      | π × 34 mm              |
| Distance per encoder tick | **0.30 mm**       | 106.8 mm ÷ 355 CPR     |
| Ticks per cell (16 cm)    | **~533 ticks**    | 160 mm ÷ 0.30 mm       |
| Cell size                 | **16 cm × 16 cm** | Per maze specification |
| Maze dimensions           | **8 × 8 cells**   | 128 cm × 128 cm total  |

---

## 3. Software Architecture

### 3.1 Layered Design

```
┌─────────────────────────────────┐
│        FLOOD FILL LAYER         │  Recalculates cell distances
│     (maze solving brain)        │
├─────────────────────────────────┤
│        MAZE MANAGER             │  Wall storage + grid state
│     (walls + visited flags)     │
├─────────────────────────────────┤
│       DECISION ENGINE           │  Picks next cell to move to
│     (direction selection)       │
├─────────────────────────────────┤
│       MOTION CONTROL            │  PID + encoder + MPU fusion
│     (accurate movement)         │
├─────────────────────────────────┤
│         HARDWARE                │  Raw sensor/motor I/O
│   (motors, encoders, sensors)   │
└─────────────────────────────────┘
```

### 3.2 File Structure

```
v2/
├── PRD.md                  # This document
├── main.ino                # Entry point, state machine, main loop
│
├── movement.h / .cpp       # moveForward(), turnLeft90(), turnRight90(), turnBack()
├── pid.h / .cpp            # PID controller for straight-line + turn correction
├── encoders.h / .cpp       # ISR-driven tick counters, distance calculation
├── sensors.h / .cpp        # Ultrasonic read + averaging, wall detection booleans
├── imu.h / .cpp            # MPU6050 init, calibration, yaw integration
├── motors.h / .cpp         # TB6612 low-level: setMotorSpeed(), brake()
│
├── maze.h / .cpp           # Cell struct, wall storage, maze initialization
├── floodfill.h / .cpp      # Flood fill algorithm, distance recalculation
├── navigation.h / .cpp     # Direction logic, path storage, speed run replay
│
└── config.h                # All pin definitions, constants, tuning parameters
```

### 3.3 Data Structures

#### Cell Structure

```cpp
struct Cell {
  uint8_t walls;      // Bitmask: bit 0=N, 1=E, 2=S, 3=W
  uint8_t distance;   // Flood fill distance value
  bool visited;       // Has robot entered this cell?
};
```

#### Maze Array

```cpp
Cell maze[8][8];      // 8×8 grid — uses ~192 bytes (fits in 2KB SRAM)
```

#### Direction Encoding

```cpp
enum Direction { NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3 };
```

#### Robot State

```cpp
struct RobotState {
  uint8_t x, y;            // Current cell position (0-7, 0-7)
  Direction heading;       // Current facing direction
  bool isExploring;        // true = explore mode, false = speed run
};
```

### 3.4 Memory Budget (ATmega328P: 2048 bytes SRAM)

| Allocation                | Bytes    | Notes                                  |
| ------------------------- | -------- | -------------------------------------- |
| Maze array (`Cell[8][8]`) | ~192     | 3 bytes × 64 cells                     |
| Flood fill queue          | ~64      | BFS working buffer                     |
| Speed run path            | ~64      | Max 64 moves stored                    |
| PID state variables       | ~32      | Floats for error, integral, derivative |
| Sensor buffers            | ~30      | 5-sample averaging × 3 sensors         |
| Stack + locals            | ~500     | Function calls, ISR stack              |
| **Total estimated**       | **~882** | **Well within 2 KB**                   |

---

## 4. Functional Requirements

### 4.1 Hardware Layer

| ID    | Requirement                                                       | Priority |
| ----- | ----------------------------------------------------------------- | -------- |
| HW-01 | Read left/right encoder ticks via hardware interrupts (D2, D3)    | P0       |
| HW-02 | Read ultrasonic distance for front, left, right sensors           | P0       |
| HW-03 | Initialize MPU6050 and read yaw angle via Z-axis gyro integration | P0       |
| HW-04 | Control motor speed and direction via TB6612FNG                   | P0       |
| HW-05 | Monitor battery voltage via voltage divider on analog pin         | P1       |
| HW-06 | Active braking (both H-bridge pins LOW) for sharp stops           | P1       |

### 4.2 Motion Control Layer

| ID    | Requirement                                                                             | Priority |
| ----- | --------------------------------------------------------------------------------------- | -------- |
| MC-01 | `moveForward(cells)` — move exact number of cells using encoder ticks (~533 ticks/cell) | P0       |
| MC-02 | `turnLeft90()` — pivot 90° left using MPU6050 yaw feedback                              | P0       |
| MC-03 | `turnRight90()` — pivot 90° right using MPU6050 yaw feedback                            | P0       |
| MC-04 | `turnBack()` — pivot 180° using MPU6050 yaw feedback                                    | P0       |
| MC-05 | Straight-line PID correction using encoder differential                                 | P0       |
| MC-06 | Heading PID using MPU6050 during forward movement                                       | P0       |
| MC-07 | Wall-centering correction when both side walls present                                  | P1       |

### 4.3 Sensor Layer

| ID    | Requirement                                                    | Priority |
| ----- | -------------------------------------------------------------- | -------- |
| SN-01 | Detect front wall (distance < threshold) → `wallFront` boolean | P0       |
| SN-02 | Detect left wall (distance < threshold) → `wallLeft` boolean   | P0       |
| SN-03 | Detect right wall (distance < threshold) → `wallRight` boolean | P0       |
| SN-04 | 5-sample averaging to filter ultrasonic noise                  | P0       |
| SN-05 | MPU6050 gyro calibration with variance validation at startup   | P1       |

### 4.4 Maze & Algorithm Layer

| ID    | Requirement                                                | Priority |
| ----- | ---------------------------------------------------------- | -------- |
| MA-01 | Initialize 8×8 maze grid with outer boundary walls         | P0       |
| MA-02 | Update maze walls from sensor readings at each cell        | P0       |
| MA-03 | Run Flood Fill from goal cells: (3,3), (3,4), (4,3), (4,4) | P0       |
| MA-04 | Select next move toward lowest-distance reachable neighbor | P0       |
| MA-05 | Detect when goal is reached                                | P0       |
| MA-06 | Store shortest path as move sequence (F/L/R) for speed run | P1       |
| MA-07 | Execute speed run by replaying stored path at higher speed | P1       |

---

## 5. PID Control Specification

### 5.1 Straight-Line Correction (Encoder-Based)

```
error = leftEncoderTicks - rightEncoderTicks
integral += error
derivative = error - lastError

correction = Kp * error + Ki * integral + Kd * derivative

leftMotor  = baseSpeed - correction
rightMotor = baseSpeed + correction
```

### 5.2 Turn Control (MPU6050-Based)

```
error = targetAngle - currentYaw       // wrapped to ±180°
correction = Kp * error

leftMotor  = +correction               // pivot turn
rightMotor = -correction
```

### 5.3 Recommended Starting Values

| Parameter  | Straight-Line | Turns  |
| ---------- | :-----------: | :----: |
| Kp         |      2.0      |  4.0   |
| Ki         |      0.0      |  0.0   |
| Kd         |      1.0      |  0.0   |
| Base Speed |    100 PWM    | 80 PWM |

### 5.4 Tuning Procedure

1. Set `Ki = 0`, `Kd = 0`
2. Increase `Kp` from 1.0 until robot oscillates → note the value
3. Set `Kp` to 60% of oscillation value
4. Add `Kd` starting at 0.5, increase until oscillation is damped
5. Add small `Ki` (0.01–0.05) only if persistent drift exists

---

## 6. Algorithm: Flood Fill

### 6.1 Overview

Flood Fill assigns a **distance value** to every cell, with the goal cells set to 0. The robot always moves to the accessible neighbor with the **lowest distance value**.

### 6.2 Goal Cells

```
(3,3)  (3,4)
(4,3)  (4,4)
```

All set to `distance = 0`.

### 6.3 Algorithm Steps

```
1. Set goal cell distances to 0
2. Add goal cells to queue
3. While queue not empty:
   a. Dequeue cell (x, y)
   b. For each neighbor (N, E, S, W):
      - If no wall between (x,y) and neighbor
      - If neighbor.distance > current.distance + 1
        → Set neighbor.distance = current.distance + 1
        → Enqueue neighbor
```

### 6.4 Distance Grid Example (No Walls)

```
7  6  5  4  4  5  6  7
6  5  4  3  3  4  5  6
5  4  3  2  2  3  4  5
4  3  2  1  1  2  3  4
4  3  2  1  1  2  3  4
5  4  3  2  0  3  4  5
6  5  4  3  3  4  5  6
7  6  5  4  4  5  6  7
```

### 6.5 Decision Logic

```
At cell (x, y) facing direction D:
  1. Read sensors → update walls for current cell
  2. Run Flood Fill to recalculate distances
  3. Check all 4 neighbors:
     - Exclude neighbors blocked by walls
     - Select neighbor with smallest distance
  4. Turn to face selected neighbor
  5. Move forward one cell
```

---

## 7. State Machine

### 7.1 Exploration Mode

```
         ┌──────────────┐
         │    STARTUP    │
         │  calibrate    │
         │  MPU6050      │
         └──────┬───────┘
                │
         ┌──────▼───────┐
    ┌──→ │  READ WALLS   │
    │    │  update maze  │
    │    └──────┬───────┘
    │           │
    │    ┌──────▼───────┐
    │    │  FLOOD FILL   │
    │    │  recalculate  │
    │    └──────┬───────┘
    │           │
    │    ┌──────▼───────┐
    │    │   DECIDE      │
    │    │  next cell    │
    │    └──────┬───────┘
    │           │
    │    ┌──────▼───────┐         ┌────────────┐
    │    │    MOVE       │────────→│  GOAL      │
    │    │  execute      │  goal? │  REACHED   │
    │    └──────┬───────┘         └─────┬──────┘
    │           │ no                    │
    └───────────┘                      │
                                ┌──────▼───────┐
                                │  SPEED RUN   │
                                │  replay path │
                                └──────────────┘
```

### 7.2 Speed Run Mode

1. Reverse Flood Fill from start → goal to get optimal path
2. Store path as a sequence of moves: `F` (forward), `L` (turn left), `R` (turn right)
3. Execute stored path at elevated `baseSpeed`
4. No sensor-based wall updates needed (maze is known)

---

## 8. Development Stages

> ⚠️ **Follow this order strictly. Do NOT skip stages.**

### Stage 1 — Hardware Validation

| Step | Test                   | Pass Criteria                                   |
| ---- | ---------------------- | ----------------------------------------------- |
| 1.1  | Motor forward/backward | Both motors spin correct directions             |
| 1.2  | Encoder tick count     | Ticks increment consistently; correct direction |
| 1.3  | MPU6050 yaw            | Angle reads ~90° on 90° hand rotation           |
| 1.4  | Ultrasonic distance    | Reads ~10 cm for object at 10 cm (±2 cm)        |
| 1.5  | Battery voltage        | ADC reading maps to correct voltage             |

### Stage 2 — Motion Control

| Step | Test                | Pass Criteria                            |
| ---- | ------------------- | ---------------------------------------- |
| 2.1  | Move 1 cell (16 cm) | Robot stops within ±1 cm of target       |
| 2.2  | Straight-line PID   | Robot stays within ±2 cm over 5-cell run |
| 2.3  | Turn 90° left       | Heading within ±3° of target             |
| 2.4  | Turn 90° right      | Heading within ±3° of target             |
| 2.5  | Turn 180°           | Heading within ±5° of target             |

### Stage 3 — Wall Detection

| Step | Test                         | Pass Criteria                                          |
| ---- | ---------------------------- | ------------------------------------------------------ |
| 3.1  | Front wall detect            | Detects wall at < 10 cm, no false positives at > 14 cm |
| 3.2  | Side wall detect             | Detects walls with ±2 cm accuracy                      |
| 3.3  | All 3 sensors simultaneously | No interference between sensors                        |

### Stage 4 — Maze Representation

| Step | Test                      | Pass Criteria                                      |
| ---- | ------------------------- | -------------------------------------------------- |
| 4.1  | Initialize maze grid      | All outer walls set, inner walls clear             |
| 4.2  | Update walls from sensors | Walls stored correctly for current cell + neighbor |
| 4.3  | Print maze over Serial    | Verify manual wall placement matches output        |

### Stage 5 — Flood Fill Algorithm

| Step | Test                            | Pass Criteria                                    |
| ---- | ------------------------------- | ------------------------------------------------ |
| 5.1  | Flood fill on empty maze        | Distance grid matches expected Manhattan pattern |
| 5.2  | Flood fill with walls           | Distances route around walls correctly           |
| 5.3  | Decision picks correct neighbor | Always selects lowest-distance open neighbor     |

### Stage 6 — Full Exploration

| Step | Test                               | Pass Criteria                    |
| ---- | ---------------------------------- | -------------------------------- |
| 6.1  | Explore simple maze (no dead-ends) | Reaches center without collision |
| 6.2  | Explore complex maze (dead-ends)   | Backtracks and finds center      |
| 6.3  | Complete exploration               | All reachable cells visited      |

### Stage 7 — Speed Run

| Step | Test           | Pass Criteria                               |
| ---- | -------------- | ------------------------------------------- |
| 7.1  | Path storage   | Correct sequence of F/L/R moves             |
| 7.2  | Path replay    | Robot completes speed run without collision |
| 7.3  | Speed increase | Faster than exploration run                 |

---

## 9. Sensor Configuration

### 9.1 Ultrasonic Sensor Placement

```
            FRONT (0°)
              ↑
          ┌────────┐
LEFT (90°)│  ROBOT │ RIGHT (-90°)
          └────────┘
```

- **Height**: 2–3 cm from floor (to catch 16 cm walls)
- **Orientation**: Perpendicular to respective wall face

### 9.2 Wall Detection Thresholds

| Condition    | Distance | Action                      |
| ------------ | -------- | --------------------------- |
| Wall present | < 10 cm  | `wallX = true`              |
| No wall      | > 14 cm  | `wallX = false`             |
| Danger zone  | < 5 cm   | Emergency stop / correction |

### 9.3 Noise Mitigation

```cpp
int readUltrasonic(int trigPin, int echoPin) {
  long sum = 0;
  for (int i = 0; i < 5; i++) {
    // trigger + read pulse
    sum += singleReading(trigPin, echoPin);
    delayMicroseconds(100);
  }
  return sum / 5;
}
```

---

## 10. Known Risks & Mitigations

| Risk                            | Impact                   | Mitigation                                                                   |
| ------------------------------- | ------------------------ | ---------------------------------------------------------------------------- |
| Ultrasonic cross-talk           | Ghost walls detected     | Sequential sensor reading (not parallel)                                     |
| Ultrasonic latency (~30 ms × 3) | Slow exploration speed   | Accept slower speed; optimize read order                                     |
| MPU6050 gyro drift              | Cumulative heading error | Use gyro **only** during turns; reset heading at known wall alignments       |
| SRAM exhaustion                 | Crashes, corruption      | Strict memory budget (see §3.4); avoid `String`; use `F()` for Serial prints |
| Encoder slip on smooth floors   | Distance errors          | Calibrate ticks-per-cell on actual surface                                   |
| A6/A7 analog-only limitation    | Echo pin incompatibility | Reassign pins or use analog threshold method                                 |

---

## 11. Testing Strategy

### 11.1 Simulator Testing (Pre-Hardware)

- **Tool**: [MMS — Mackorone's Micromouse Simulator](https://github.com/mackorone/mms)
- **Purpose**: Validate Flood Fill + navigation logic before deploying to robot
- **API**: Simulator communicates via `stdin/stdout` with commands: `moveForward`, `turnLeft`, `turnRight`, `wallFront`, `wallLeft`, `wallRight`

### 11.2 Unit Tests (Per Module)

Each hardware module has its own test sketch:

```
tests/
├── test_motors.ino
├── test_encoders.ino
├── test_mpu6050.ino
├── test_ultrasonic.ino
└── test_battery.ino
```

### 11.3 Integration Tests

| Test             | Description                                       |
| ---------------- | ------------------------------------------------- |
| Move-and-detect  | Move 1 cell, read walls, verify correct detection |
| Turn-and-detect  | Turn 90°, read walls, verify new orientation      |
| Exploration loop | Full explore → center in known simple maze        |

---

## 12. Competition-Readiness Checklist

- [ ] Robot fits within 25 cm × 25 cm footprint
- [ ] Battery fully charged before each run
- [ ] MPU6050 calibration completes without variance rejection
- [ ] Encoder ticks-per-cell calibrated on competition surface
- [ ] Wall detection thresholds tuned for competition wall material
- [ ] Flood Fill tested on at least 3 different maze configurations
- [ ] Speed run completes without collision at elevated speed
- [ ] Emergency stop button functional

---

## 13. Glossary

| Term           | Definition                                                      |
| -------------- | --------------------------------------------------------------- |
| **CPR**        | Counts Per Revolution — encoder resolution (355 for our motors) |
| **Flood Fill** | BFS-based shortest path algorithm for grid mazes                |
| **PID**        | Proportional-Integral-Derivative — feedback control loop        |
| **PWM**        | Pulse Width Modulation — motor speed control (0–255 on Nano)    |
| **ISR**        | Interrupt Service Routine — hardware interrupt handler          |
| **IMU**        | Inertial Measurement Unit — MPU6050 gyro + accelerometer        |
| **BFS**        | Breadth-First Search — algorithm used by Flood Fill             |
| **SRAM**       | Static RAM — 2 KB working memory on ATmega328P                  |
| **Cell**       | One square unit of the maze (16 cm × 16 cm)                     |
| **Speed Run**  | Second traversal at maximum speed using memorized path          |
