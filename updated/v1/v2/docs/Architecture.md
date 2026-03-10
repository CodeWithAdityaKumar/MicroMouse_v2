# Micromouse v1 — Software Architecture

> **Version:** 1.0  
> **Date:** 2026-03-02  
> **Platform:** Arduino Nano (ATmega328P) · PlatformIO

---

## 1. High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      APPLICATION LAYER                       │
│                                                             │
│   ┌──────────────┐   ┌──────────┐   ┌──────────────────┐   │
│   │  State Machine│   │ Flood    │   │  Path Planner    │   │
│   │  Controller   │   │ Fill     │   │  (Speed Run)     │   │
│   └──────┬───────┘   └────┬─────┘   └────────┬─────────┘   │
│          │                │                   │             │
├──────────┼────────────────┼───────────────────┼─────────────┤
│          │         CONTROL LAYER              │             │
│   ┌──────┴───────┐   ┌────┴─────┐   ┌────────┴─────────┐   │
│   │  Motion      │   │  Maze    │   │  Navigation      │   │
│   │  Controller  │   │  Manager │   │  Engine          │   │
│   └──────┬───────┘   └──────────┘   └──────────────────┘   │
│          │                                                  │
├──────────┼──────────────────────────────────────────────────┤
│          │            DRIVER LAYER (HAL)                     │
│   ┌──────┴───────┐ ┌──────────┐ ┌────────┐ ┌────────────┐  │
│   │  Motor       │ │  IMU     │ │ IR     │ │ Encoder    │  │
│   │  Driver      │ │  Driver  │ │ Sensor │ │ Driver     │  │
│   │  (TB6612)    │ │ (MPU6050)│ │ Driver │ │ (Interrupt)│  │
│   └──────────────┘ └──────────┘ └────────┘ └────────────┘  │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│                      HARDWARE (MCU + Peripherals)           │
│   Arduino Nano · ATmega328P · 16 MHz · 2 KB SRAM · 32 KB   │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. Module Breakdown

### 2.1 Driver Layer (HAL — Hardware Abstraction)

Each driver is a `.h` / `.cpp` pair. All hardware register access is isolated here.

| Module          | Files                       | Responsibility                                                                                                                                       |
| --------------- | --------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------- |
| **MotorDriver** | `motor.h` / `motor.cpp`     | PWM output (D5, D6), direction pins (D9–D12), standby (D13). Functions: `setMotors(leftPWM, rightPWM)`, `stopMotors()`, `enableMotors()`.            |
| **IMU**         | `imu.h` / `imu.cpp`         | I²C init, gyro calibration (bias removal), raw Z-axis read, yaw integration. Functions: `initIMU()`, `calibrateGyro()`, `updateYaw(dt)`, `getYaw()`. |
| **IRSensors**   | `sensors.h` / `sensors.cpp` | Analog reads (A0, A1, A2), ADC → cm conversion, multi-sample averaging. Functions: `readWalls()` → `WallReadings { left, front, right }`.            |
| **Encoders**    | `encoder.h` / `encoder.cpp` | ISR on D2 (INT0) and D3 (INT1), direction from D7 / D8, tick counters. Functions: `getLeftTicks()`, `getRightTicks()`, `resetTicks()`.               |
| **Button**      | `button.h` / `button.cpp`   | Debounced read on D4 (INPUT_PULLUP). Short press / long press detection.                                                                             |

### 2.2 Control Layer

| Module               | Files                             | Responsibility                                                                                                                                    |
| -------------------- | --------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| **PID**              | `pid.h` / `pid.cpp`               | Generic PID class with windup guard and output clamp. Instances: `headingPID`, `wallPID`.                                                         |
| **MotionController** | `motion.h` / `motion.cpp`         | Combines heading PID + wall PID → differential motor output. Manages cell-advance (encoder tick counting), turn execution (gyro-guided 90°/180°). |
| **MazeManager**      | `maze.h` / `maze.cpp`             | 16×16 cell array, wall storage, flood value storage. Functions: `setWall()`, `getFloodValue()`, `markVisited()`.                                  |
| **Navigation**       | `navigation.h` / `navigation.cpp` | Implements flood fill BFS, path computation, next-move decision. Functions: `floodFill()`, `getNextDirection()`.                                  |

### 2.3 Application Layer

| Module           | Files                                   | Responsibility                                                                                             |
| ---------------- | --------------------------------------- | ---------------------------------------------------------------------------------------------------------- |
| **StateMachine** | `state_machine.h` / `state_machine.cpp` | Manages global state transitions. Orchestrates calibration → exploration → return → speed run.             |
| **main.cpp**     | `src/main.cpp`                          | `setup()` and `loop()`. Initialises all modules, runs the 200 Hz control loop, delegates to state machine. |
| **Config**       | `config.h`                              | All tunable constants: pin definitions, PID gains, speed values, thresholds, maze dimensions.              |

---

## 3. File Structure

```
v1/
├── platformio.ini
├── src/
│   └── main.cpp                 # Entry point
├── include/
│   └── config.h                 # Pin defs, constants, tuning
├── lib/
│   ├── Motor/
│   │   ├── motor.h
│   │   └── motor.cpp
│   ├── IMU/
│   │   ├── imu.h
│   │   └── imu.cpp
│   ├── Sensors/
│   │   ├── sensors.h
│   │   ├── sensors.cpp
│   │   └── WallReadings.h       # Shared struct
│   ├── Encoder/
│   │   ├── encoder.h
│   │   └── encoder.cpp
│   ├── Button/
│   │   ├── button.h
│   │   └── button.cpp
│   ├── PID/
│   │   ├── pid.h
│   │   └── pid.cpp
│   ├── Motion/
│   │   ├── motion.h
│   │   └── motion.cpp
│   ├── Maze/
│   │   ├── maze.h
│   │   └── maze.cpp
│   ├── Navigation/
│   │   ├── navigation.h
│   │   └── navigation.cpp
│   └── StateMachine/
│       ├── state_machine.h
│       └── state_machine.cpp
├── docs/
│   ├── Pins.md
│   ├── PRD.md
│   ├── Architecture.md          # ← this file
│   ├── System Design.md
│   └── MVP Tech Docs.md
└── test/
    └── (unit test stubs)
```

---

## 4. Data Flow

```
┌──────────┐    ┌──────────┐    ┌──────────────┐
│ IR Sens. ├───►│ Wall     ├───►│ Maze Manager │
│ (analog) │    │ Classify │    │ (wall store) │
└──────────┘    └──────────┘    └──────┬───────┘
                                       │
┌──────────┐    ┌──────────┐    ┌──────▼───────┐    ┌──────────┐
│ Encoders ├───►│ Cell     ├───►│ Flood Fill   ├───►│ Next     │
│ (ticks)  │    │ Counter  │    │ (BFS)        │    │ Decision │
└──────────┘    └──────────┘    └──────────────┘    └────┬─────┘
                                                         │
┌──────────┐    ┌──────────┐                      ┌──────▼─────┐
│ MPU6050  ├───►│ Yaw      ├─────────────────────►│ Motion     │
│ (gyro Z) │    │ Integr.  │                      │ Controller │
└──────────┘    └──────────┘                      └──────┬─────┘
                                                         │
                                                  ┌──────▼─────┐
                                                  │ Motor PWM  │
                                                  │ Output     │
                                                  └────────────┘
```

---

## 5. State Machine Diagram

```
                         ┌─────────────┐
                         │  POWER ON   │
                         └──────┬──────┘
                                │
                         ┌──────▼──────┐
                    ┌───►│  STATE_IDLE │◄────────────────┐
                    │    └──────┬──────┘                  │
                    │           │ button press            │
                    │    ┌──────▼──────────┐              │
                    │    │ CALIBRATING     │              │
                    │    │ (gyro bias cal) │              │
                    │    └──────┬──────────┘              │
                    │           │ done                    │
                    │    ┌──────▼──────────┐              │
                    │    │ HEADING_LOCK    │              │
                    │    │ (yaw = 0°)     │              │
                    │    └──────┬──────────┘              │
                    │           │ button press            │
                    │    ┌──────▼──────────┐              │
                    │    │ EXPLORING       │              │
                    │    │ (flood fill)    │──┐           │
                    │    └──────┬──────────┘  │ turn      │
                    │           │ goal found  │ needed    │
                    │    ┌──────▼──────────┐  │           │
                    │    │ RETURNING       │◄─┘           │
                    │    │ (back to start) │              │
                    │    └──────┬──────────┘              │
                    │           │ at (0,0)               │
                    │    ┌──────▼──────────┐              │
                    │    │ READY_SPEED_RUN │              │
                    │    └──────┬──────────┘              │
                    │           │ button press            │
                    │    ┌──────▼──────────┐              │
                    │    │ SPEED_RUN       │              │
                    │    └──────┬──────────┘              │
                    │           │ goal reached            │
                    │    ┌──────▼──────────┐              │
                    │    │ DONE            │              │
                    │    └─────────────────┘              │
                    │                                    │
                    │    ┌─────────────────┐              │
                    └────│ STOPPED         │──────────────┘
                         │ (long press)    │
                         └─────────────────┘
```

---

## 6. Interrupt Architecture

| Interrupt | Vector               | Pin | Handler                                                                          |
| --------- | -------------------- | --- | -------------------------------------------------------------------------------- |
| INT0      | External Interrupt 0 | D2  | `leftEncoderISR()` — Reads D7 for direction, increments/decrements `leftTicks`   |
| INT1      | External Interrupt 1 | D3  | `rightEncoderISR()` — Reads D8 for direction, increments/decrements `rightTicks` |

> **Rule:** ISRs only modify `volatile` tick counters. No I²C, no Serial, no floating-point inside ISRs.

---

## 7. Memory Budget (2 KB SRAM)

| Component                            | Bytes      | Notes                               |
| ------------------------------------ | ---------- | ----------------------------------- |
| Maze array (16×16 × 2 bytes)         | 512        | walls + flood value per cell        |
| BFS queue (max 256 entries × 1 byte) | 256        | Circular buffer for flood fill      |
| PID state (2 instances × 24 bytes)   | 48         | error, integral, derivative, output |
| Yaw + sensor state                   | 32         | floats for angles, distances        |
| Stack + locals                       | ~700       | Function call overhead              |
| Serial TX buffer                     | 64         | UART output (debug only)            |
| **Total estimated**                  | **~1,612** | Leaves ~400 bytes headroom          |

---

## 8. Timing Budget (5 ms Loop)

| Operation                   | Time (μs)  | Notes                                   |
| --------------------------- | ---------- | --------------------------------------- |
| IMU read (I²C)              | ~800       | 6 bytes at 400 kHz                      |
| Gyro integration            | ~20        | float multiply + add                    |
| IR sensor read (3×)         | ~350       | 3 × analogRead (~110 μs each)           |
| Wall classification         | ~10        | threshold compare                       |
| PID compute (2×)            | ~40        | heading + wall                          |
| Motor PWM write             | ~10        | analogWrite × 2                         |
| Flood fill (worst case)     | ~2,000     | Full 16×16 BFS (only on cell change)    |
| State machine logic         | ~50        | branching + transitions                 |
| **Total (no flood fill)**   | **~1,280** | Comfortable within 5 ms                 |
| **Total (with flood fill)** | **~3,280** | OK — flood fill only runs once per cell |

---

## 9. Communication Interfaces

| Bus  | Speed               | Devices                                    |
| ---- | ------------------- | ------------------------------------------ |
| I²C  | 400 kHz (Fast Mode) | MPU6050 (addr 0x68)                        |
| UART | 115200 baud         | Debug serial (D0/D1)                       |
| GPIO | —                   | Motor driver, encoders, button, IR sensors |

---

## 10. Dependency Graph

```
main.cpp
├── config.h
├── StateMachine
│   ├── Navigation
│   │   ├── Maze
│   │   └── FloodFill (within Navigation)
│   ├── MotionController
│   │   ├── PID (heading)
│   │   ├── PID (wall centering)
│   │   ├── MotorDriver
│   │   ├── IMU
│   │   ├── IRSensors
│   │   └── Encoders
│   └── Button
└── Arduino.h / Wire.h
```

---

## 11. Build & Flash

```bash
# Build
pio run -e nanoatmega328

# Upload
pio run -e nanoatmega328 --target upload

# Serial Monitor
pio device monitor -b 115200
```

---

## 12. Design Decisions

| Decision                            | Rationale                                                                  |
| ----------------------------------- | -------------------------------------------------------------------------- |
| **PlatformIO over Arduino IDE**     | Proper library management, multi-file builds, no prototype mangling issues |
| **Separate WallReadings.h struct**  | Prevents Arduino IDE prototype generation errors (learned from prior bugs) |
| **200 Hz loop, not faster**         | Balances I²C read time + analog reads within tight 5 ms window             |
| **BFS only on cell transition**     | Flood fill is O(256) worst case — too expensive for every loop iteration   |
| **Pivot turns only (no curves)**    | Simpler control for v1; curves planned for v2                              |
| **Float for yaw, uint8_t for maze** | Yaw needs precision; maze data must be compact for SRAM                    |
