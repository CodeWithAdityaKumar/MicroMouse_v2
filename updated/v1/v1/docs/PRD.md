# Micromouse v1 — Product Requirements Document (PRD)

> **Version:** 1.0  
> **Date:** 2026-03-02  
> **Platform:** Arduino Nano (ATmega328P) · PlatformIO  
> **Algorithm:** Flood Fill

---

## 1. Executive Summary

Build an **industry-grade autonomous micromouse** capable of solving a standard 16×16 maze using the **Flood Fill algorithm**. The robot must explore the maze, map wall positions, compute the shortest path from start (0,0) to the centre goal (7,7 / 7,8 / 8,7 / 8,8), and execute a speed-optimised re-run — all within a single battery charge.

---

## 2. Problem Statement

Classic micromouse competitions require a robot to:

1. Navigate an unknown 16×16 cell maze autonomously.
2. Discover and memorise wall positions during exploration.
3. Compute the optimal path to the goal zone.
4. Execute a fast "speed run" along the optimal path.

The robot must do this **without any external input** — no remote control, no pre-loaded maps, no GPS. All decisions are made on-board in real-time.

---

## 3. Goals & Success Criteria

| #   | Goal                               | Success Metric                                                                            |
| --- | ---------------------------------- | ----------------------------------------------------------------------------------------- |
| G1  | **Complete maze exploration**      | Robot visits every reachable cell or identifies the optimal path without full exploration |
| G2  | **Correct flood fill pathfinding** | Robot consistently reaches the 4-cell centre goal                                         |
| G3  | **Reliable wall detection**        | ≥ 95% wall classification accuracy (wall / no-wall)                                       |
| G4  | **Straight-line driving**          | Heading deviation < ±2° over 1 cell (18 cm) with PID correction                           |
| G5  | **Accurate 90° and 180° turns**    | Turn error < ±3° measured by gyro                                                         |
| G6  | **Speed run execution**            | Robot traverses the optimal path ≥ 30% faster than exploration speed                      |
| G7  | **Encoder-based odometry**         | Cell-boundary detection accurate within ±5 mm                                             |
| G8  | **Robust state machine**           | Zero unrecoverable hangs; button-triggered abort always works                             |

---

## 4. Hardware Bill of Materials

| Component         | Model / Spec                            | Qty | Role                                |
| ----------------- | --------------------------------------- | --- | ----------------------------------- |
| Microcontroller   | Arduino Nano (ATmega328P, 16 MHz)       | 1   | Brain                               |
| Motor Driver      | TB6612FNG Dual H-Bridge                 | 1   | Motor control                       |
| DC Motors         | N20 micro gear motors with encoders     | 2   | Locomotion                          |
| IMU               | MPU6050 (gyroscope + accelerometer)     | 1   | Heading / yaw tracking              |
| Wall Sensors      | Sharp GP2Y0A21YK0F (10–80 cm) analog IR | 3   | Left / Front / Right wall detection |
| Push Button       | Tactile switch (NO)                     | 1   | Start / calibrate / abort           |
| Battery           | 7.4V 2S LiPo (300–500 mAh)              | 1   | Power supply                        |
| Voltage Regulator | LM7805 or buck converter (→ 5V)         | 1   | Logic-level supply                  |
| Chassis           | Custom PCB or 3D-printed body           | 1   | Structural                          |
| Wheels            | Silicone / rubber, ~30 mm diameter      | 2   | Traction                            |
| Caster / Ball     | 8 mm ball caster                        | 1   | Third point of contact              |

> **Pin mapping:** see [Pins.md](./Pins.md)

---

## 5. Functional Requirements

### FR-1: Initialisation & Calibration

| ID     | Requirement                                                                                                 |
| ------ | ----------------------------------------------------------------------------------------------------------- |
| FR-1.1 | On power-up, enter `STATE_IDLE` — motors off, LED on D13 blinks slowly.                                     |
| FR-1.2 | On button press (D4), enter `STATE_CALIBRATING` — MPU6050 gyro bias calibrated over 2 seconds (stationary). |
| FR-1.3 | Post-calibration, lock current yaw as heading 0° (`STATE_HEADING_LOCK`).                                    |
| FR-1.4 | Second button press transitions to `STATE_EXPLORING`.                                                       |

### FR-2: Wall Sensing

| ID     | Requirement                                                                                                                 |
| ------ | --------------------------------------------------------------------------------------------------------------------------- |
| FR-2.1 | Read 3 Sharp IR sensors (A0, A1, A2) and convert ADC values → distance (cm) using calibrated lookup or linearisation curve. |
| FR-2.2 | Classify each side as `WALL` or `NO_WALL` using a threshold (configurable, default ~6 cm for cell wall presence).           |
| FR-2.3 | Update the maze map's wall data for the current cell and its neighbour (walls are shared between adjacent cells).           |
| FR-2.4 | Sensor readings averaged over 4 samples to reject noise.                                                                    |

### FR-3: Motion Control

| ID     | Requirement                                                                                                                                             |
| ------ | ------------------------------------------------------------------------------------------------------------------------------------------------------- |
| FR-3.1 | **Drive Straight:** PID controller on heading error (gyro yaw). Correction applied as differential motor speed.                                         |
| FR-3.2 | **Wall Centering:** When both left and right walls present, add secondary PID term using IR sensor delta for centering.                                 |
| FR-3.3 | **Turn 90°:** Pivot-in-place turn using gyro. Wait for heading to settle within ±1° of target before resuming.                                          |
| FR-3.4 | **Turn 180°:** Same as 90° but for 180° delta.                                                                                                          |
| FR-3.5 | **Cell Counting:** Left and right encoders track ticks. One cell = known tick count (calibrated). Cell boundary triggers wall read + flood fill recalc. |
| FR-3.6 | Motor speed controlled via PWM on D5 (left) and D6 (right) through TB6612FNG.                                                                           |

### FR-4: Maze Mapping & Flood Fill

| ID     | Requirement                                                                                                             |
| ------ | ----------------------------------------------------------------------------------------------------------------------- |
| FR-4.1 | Maintain a 16×16 array of cell structs: `{ walls[4], visited, floodValue }`.                                            |
| FR-4.2 | Implement **Flood Fill algorithm**: BFS from goal cells outward, assigning distance values.                             |
| FR-4.3 | After each cell transition, update walls and re-run flood fill from the goal.                                           |
| FR-4.4 | Decision logic: move to the accessible neighbour with the **lowest flood value**.                                       |
| FR-4.5 | Exploration ends when the robot reaches any goal cell AND has a confident shortest path (or full exploration complete). |

### FR-5: Speed Run

| ID     | Requirement                                                                                      |
| ------ | ------------------------------------------------------------------------------------------------ |
| FR-5.1 | After exploration, robot returns to start (0,0) using flood fill (goal = start).                 |
| FR-5.2 | On button press at start, execute speed run: follow the pre-computed optimal path at higher PWM. |
| FR-5.3 | Speed run uses the stored wall map (no new exploration needed).                                  |
| FR-5.4 | Increased base speed with tighter PID tuning.                                                    |

### FR-6: State Machine

```
STATE_IDLE → (button) → STATE_CALIBRATING → (done) → STATE_HEADING_LOCK
  → (button) → STATE_EXPLORING → (goal reached) → STATE_RETURNING
  → (at start) → STATE_READY_SPEED_RUN → (button) → STATE_SPEED_RUN
  → (goal reached) → STATE_DONE

Any state → (button long-press) → STATE_STOPPED
STATE_STOPPED → (button) → STATE_IDLE
```

### FR-7: Safety & Recovery

| ID     | Requirement                                                                              |
| ------ | ---------------------------------------------------------------------------------------- |
| FR-7.1 | Button long-press (>1 s) triggers immediate motor stop from any state.                   |
| FR-7.2 | If gyro error exceeds ±45° from expected heading, stop motors and enter `STATE_STOPPED`. |
| FR-7.3 | If front sensor reads < 3 cm, immediate stop (collision avoidance).                      |
| FR-7.4 | Watchdog: if main loop stalls > 50 ms, trigger software reset.                           |

---

## 6. Non-Functional Requirements

| ID    | Requirement                                                                                                 |
| ----- | ----------------------------------------------------------------------------------------------------------- |
| NFR-1 | **Loop Rate:** Main control loop runs at **200 Hz** (5 ms period) using `micros()` timing — no `delay()`.   |
| NFR-2 | **Memory:** Maze array + flood fill fits within 2 KB SRAM. 16×16 cells × 2 bytes ≤ 512 bytes.               |
| NFR-3 | **Modularity:** Code split into separate `.h`/`.cpp` files per subsystem (motors, sensors, imu, maze, pid). |
| NFR-4 | **Serial Debug:** 115200 baud debug output on D0/D1 (disabled in competition mode).                         |
| NFR-5 | **Power:** Full exploration + speed run completes within 5 minutes on a single LiPo charge.                 |
| NFR-6 | **Competition Compliant:** Robot fits within 25 cm × 25 cm footprint.                                       |

---

## 7. Maze Specification (IEEE Standard)

| Parameter        | Value                                      |
| ---------------- | ------------------------------------------ |
| Grid             | 16 × 16 cells                              |
| Cell size        | 18 cm × 18 cm                              |
| Wall height      | 5 cm                                       |
| Wall thickness   | 1.2 cm                                     |
| Start cell       | (0, 0) — bottom-left corner                |
| Goal zone        | Centre 4 cells: (7,7), (7,8), (8,7), (8,8) |
| Outer walls      | All 4 perimeter walls always present       |
| Start cell walls | East wall always present (standard config) |

---

## 8. Flood Fill Algorithm — Specification

### 8.1 Data Structures

```
Cell {
  uint8_t walls;      // 4 bits: N, E, S, W (1 = wall present)
  bool    visited;     // true after robot has physically entered
  uint8_t floodValue;  // BFS distance from goal (0 at goal, 255 = unreachable)
}

Maze: Cell maze[16][16];
```

### 8.2 Algorithm Steps

1. **Initialise:** Set all flood values to 255. Set goal cells to 0.
2. **BFS Queue:** Enqueue all goal cells.
3. **Expand:** For each cell in queue, check all 4 neighbours:
   - If no wall between current and neighbour AND neighbour.floodValue > current.floodValue + 1:
     - Set neighbour.floodValue = current.floodValue + 1
     - Enqueue neighbour.
4. **Decision:** Robot moves to the adjacent accessible cell with the lowest flood value.
5. **Re-run:** After each cell transition with new wall data, re-flood from goal.

### 8.3 Exploration Strategy

- **Modified Flood Fill:** Assume unknown walls are "open" (optimistic).
- Robot tends toward the goal, discovering walls along the way.
- When blocked, flood fill recalculates and reroutes.
- Exploration terminates when the robot has an unambiguous shortest path.

---

## 9. PID Controller Specifications

### 9.1 Heading PID (Gyro-Based)

| Parameter             | Default | Range     |
| --------------------- | ------- | --------- |
| Kp                    | 3.0     | 1.0 – 8.0 |
| Ki                    | 0.01    | 0.0 – 0.1 |
| Kd                    | 0.5     | 0.1 – 2.0 |
| Integral windup limit | ±50     | —         |
| Output clamp          | ±60 PWM | —         |

### 9.2 Wall Centering PID (IR-Based)

| Parameter | Default | Range      |
| --------- | ------- | ---------- |
| Kp        | 2.0     | 0.5 – 5.0  |
| Ki        | 0.0     | 0.0 – 0.05 |
| Kd        | 0.3     | 0.0 – 1.0  |

### 9.3 Combined Correction

```
correction = headingPID(gyroError) + wallPID(sensorDelta)
leftMotor  = BASE_SPEED + correction
rightMotor = BASE_SPEED - correction
```

---

## 10. Milestones & Development Phases

| Phase  | Milestone                  | Deliverable                                                 |
| ------ | -------------------------- | ----------------------------------------------------------- |
| **P0** | Hardware assembly & wiring | Verified pin connections per Pins.md                        |
| **P1** | Sensor bring-up            | Individual test sketches for IMU, encoders, IR sensors      |
| **P2** | Motor control              | Drive straight with PID heading correction                  |
| **P3** | Turn execution             | Reliable 90° and 180° pivot turns                           |
| **P4** | Cell navigation            | Drive exactly one cell, detect cell boundaries via encoders |
| **P5** | Wall mapping               | Read/classify/store walls per cell                          |
| **P6** | Flood fill integration     | Explore maze, build map, reach the goal                     |
| **P7** | Return & speed run         | Return to start, execute speed run                          |
| **P8** | Tuning & competition prep  | PID tuning, speed optimisation, reliability testing         |

---

## 11. Risks & Mitigations

| Risk                   | Impact                      | Mitigation                                              |
| ---------------------- | --------------------------- | ------------------------------------------------------- |
| MPU6050 gyro drift     | Heading error accumulates   | Periodic wall-alignment correction + drift compensation |
| SRAM overflow          | Crash / undefined behaviour | Compact data structures, use PROGMEM for constants      |
| IR sensor noise        | False wall readings         | Multi-sample averaging + threshold hysteresis           |
| Encoder slip           | Inaccurate cell counting    | Cross-validate with IR front sensor for wall proximity  |
| Motor stall at low PWM | Robot gets stuck            | Minimum PWM threshold, stall detection via encoders     |
| Battery sag under load | Inconsistent motor speed    | Voltage divider on A3 for battery monitoring (future)   |

---

## 12. Out of Scope (v1)

- Diagonal moves / smooth curves (straight + pivot only)
- Wireless telemetry (Bluetooth / Wi-Fi)
- Multi-run optimisation with persistent EEPROM map
- Dynamic speed profiling (acceleration / deceleration curves)
- Display or LED feedback beyond D13

---

## 13. References

- [IEEE Micromouse Competition Rules](https://ieee-msite.org)
- [Flood Fill Algorithm — Micromouse Wiki](https://en.wikipedia.org/wiki/Micromouse)
- [TB6612FNG Datasheet](https://www.sparkfun.com/datasheets/Robotics/TB6612FNG.pdf)
- [MPU6050 Register Map](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf)
- [Sharp GP2Y0A21YK0F Datasheet](https://global.sharp/products/device/lineup/data/pdf/datasheet/gp2y0a21yk_e.pdf)
