# Micromouse v1 — System Design

> **Version:** 1.0  
> **Date:** 2026-03-02  
> **Platform:** Arduino Nano (ATmega328P, 16 MHz, 2 KB SRAM, 32 KB Flash)

---

## 1. System Overview

The Micromouse is a **closed-loop robotic system** that senses its environment, builds an internal map, computes optimal paths, and actuates motors — all within a tight real-time control loop on a single ATmega328P.

```
               ┌─────────────────────────────────┐
               │         ENVIRONMENT              │
               │       (16×16 Maze)               │
               └──────┬──────────────┬────────────┘
                      │              │
              reflected IR      physical contact
                      │          (wheels on floor)
                      ▼              ▼
              ┌──────────────┐ ┌──────────────┐
              │  IR Sensors  │ │  Encoders    │
              │  (3× analog) │ │  (2× interr.)│
              └──────┬───────┘ └──────┬───────┘
                     │                │
                     ▼                ▼
              ┌─────────────────────────────────┐
              │      PERCEPTION SUBSYSTEM       │
              │  • Wall classification          │
              │  • Yaw angle (MPU6050 gyro)     │
              │  • Odometry (encoder ticks)     │
              └──────────────┬──────────────────┘
                             │
                             ▼
              ┌─────────────────────────────────┐
              │      COGNITION SUBSYSTEM        │
              │  • Maze model (16×16 array)     │
              │  • Flood fill (BFS pathfinding) │
              │  • State machine (exploration   │
              │    strategy & mode control)     │
              └──────────────┬──────────────────┘
                             │
                             ▼
              ┌─────────────────────────────────┐
              │      ACTUATION SUBSYSTEM        │
              │  • PID heading controller       │
              │  • PID wall centering           │
              │  • Motor PWM output (TB6612FNG) │
              └─────────────────────────────────┘
```

---

## 2. Subsystem Design

### 2.1 Perception Subsystem

#### 2.1.1 Wall Detection (IR Sensors)

```
 Sensor Placement (top view):
  ┌─────────────────────┐
  │                     │
  │  [LEFT]   [FRONT]   │
  │  A0        A1       │
  │                     │
  │             [RIGHT] │
  │              A2     │
  │                     │
  │    ◄── Direction    │
  └─────────────────────┘
```

| Parameter         | Value                            |
| ----------------- | -------------------------------- |
| Sensor model      | Sharp GP2Y0A21YK0F (10–80 cm)    |
| ADC resolution    | 10-bit (0–1023)                  |
| Sampling          | 4-sample moving average per read |
| Wall threshold    | < 12 cm → `WALL_PRESENT`         |
| No-wall threshold | > 15 cm → `NO_WALL`              |
| Hysteresis band   | 12–15 cm → hold previous state   |

**ADC → Distance Conversion:**

```cpp
// Inverse-proportional model for Sharp GP2Y0A21YK0F
float adcToDistance(int adc) {
    if (adc < 10) return 80.0;  // clamp
    return 2076.0 / (adc - 11.0);  // calibrated for 5V Nano
}
```

#### 2.1.2 Heading Tracking (MPU6050)

| Parameter        | Value                                             |
| ---------------- | ------------------------------------------------- |
| Axis used        | Z-axis gyroscope (yaw rate)                       |
| Sample rate      | 200 Hz (matched to main loop)                     |
| Calibration      | 500-sample bias average at startup (stationary)   |
| Integration      | Trapezoidal: `yaw += (gyroZ - bias) × dt`         |
| Drift mitigation | Wall alignment correction when both walls visible |

**Drift Compensation Logic:**

```
if (leftWall && rightWall):
    sensorDelta = leftDistance - rightDistance
    if |sensorDelta| < 1.0 cm:
        // Robot is centred → correct gyro to nearest cardinal
        yaw = round(yaw / 90.0) * 90.0  // snap to grid
```

#### 2.1.3 Odometry (Encoders)

| Parameter     | Value                                         |
| ------------- | --------------------------------------------- |
| Encoder type  | Quadrature (A + B channels)                   |
| Resolution    | ~12 CPR (counts per revolution) \* gear ratio |
| Interrupts    | INT0 (D2) = left A, INT1 (D3) = right A       |
| Direction     | Read B channel (D7/D8) in ISR                 |
| Cell distance | Calibrated tick count for 18 cm               |

**ISR Design:**

```cpp
volatile long leftTicks = 0;
volatile long rightTicks = 0;

void leftEncoderISR() {
    if (digitalRead(ENC_LEFT_B)) leftTicks++;
    else leftTicks--;
}

void rightEncoderISR() {
    if (digitalRead(ENC_RIGHT_B)) rightTicks++;
    else rightTicks--;
}
```

---

### 2.2 Cognition Subsystem

#### 2.2.1 Maze Data Model

```
Memory Layout (512 bytes total):

Cell struct (2 bytes per cell):
┌─────────────────────────────────┐
│ Byte 0: walls (4 bits) +       │
│         visited (1 bit) +      │
│         reserved (3 bits)      │
│ Byte 1: floodValue (0–255)     │
└─────────────────────────────────┘

Wall encoding (4 bits):
  bit 3: NORTH wall
  bit 2: EAST wall
  bit 1: SOUTH wall
  bit 0: WEST wall

Coordinate system:
  (0,0) = bottom-left (start)
  (15,15) = top-right
  North = +Y direction
```

#### 2.2.2 Flood Fill Engine

```
Algorithm: Modified BFS (Breadth-First Search)

Input:  maze[16][16] with known walls
Output: floodValue for every cell (distance from goal)

Steps:
  1. Set all cells' floodValue = 255
  2. Set goal cells (7,7), (7,8), (8,7), (8,8) to 0
  3. Enqueue goal cells into BFS queue
  4. While queue not empty:
       cell = dequeue()
       For each direction (N, E, S, W):
         If no wall between cell and neighbour:
           If neighbour.floodValue > cell.floodValue + 1:
             neighbour.floodValue = cell.floodValue + 1
             enqueue(neighbour)
  5. Return

BFS Queue: uint8_t circular buffer, 256 entries max
  - Each entry stores cell index (row * 16 + col)
  - head/tail pointers with modulo wrap
```

**Decision Logic (per cell transition):**

```
1. Read walls → update maze[row][col].walls
2. Run floodFill() from goal
3. Check all 4 neighbour cells:
   - Must not have wall blocking
   - Select neighbour with LOWEST floodValue
4. Rotate to face that direction
5. Drive forward one cell
```

#### 2.2.3 Exploration Strategy

```
Optimistic Exploration:
  - Unknown walls treated as OPEN
  - Robot tends toward goal naturally
  - When blocked, flood fill reroutes
  - Converges much faster than wall-following

Termination Condition:
  - Robot has reached a goal cell at least once
  - Path from (0,0) to goal consists entirely of visited cells
  - OR: all cells along shortest path have confirmed walls
```

---

### 2.3 Actuation Subsystem

#### 2.3.1 Motor Control (TB6612FNG)

```
Motor Truth Table:
┌──────┬──────┬─────────────┐
│ IN1  │ IN2  │ Mode        │
├──────┼──────┼─────────────┤
│ HIGH │ LOW  │ Forward     │
│ LOW  │ HIGH │ Reverse     │
│ LOW  │ LOW  │ Coast       │
│ HIGH │ HIGH │ Brake       │
└──────┴──────┴─────────────┘

STBY (D13): HIGH = enabled, LOW = all motors off
PWM range: 0–255 (analogWrite)
Min effective PWM: ~60 (below this, motors stall)
```

#### 2.3.2 PID Control Loop

```
┌──────────┐     error      ┌──────────┐   correction   ┌──────────┐
│ Target   ├────────────────►│ Heading  ├───────────────►│ Motor    │
│ Heading  │                 │ PID      │                │ Mixer    │
└──────────┘                 └──────────┘                └────┬─────┘
                                                              │
     ┌──────────┐  sensorDelta  ┌──────────┐  wall_corr      │
     │ IR Left  ├──────────────►│ Wall     ├─────────────────►│
     │ IR Right │               │ PID      │                  │
     └──────────┘               └──────────┘                  │
                                                              ▼
                                                    ┌─────────────────┐
                                                    │ leftPWM =       │
                                                    │   base + total  │
                                                    │ rightPWM =      │
                                                    │   base - total  │
                                                    └─────────────────┘
```

**PID Implementation:**

```cpp
class PIDController {
    float Kp, Ki, Kd;
    float integral, prevError;
    float integralLimit, outputLimit;

    float compute(float error, float dt) {
        integral += error * dt;
        integral = constrain(integral, -integralLimit, integralLimit);

        float derivative = (error - prevError) / dt;
        prevError = error;

        float output = Kp * error + Ki * integral + Kd * derivative;
        return constrain(output, -outputLimit, outputLimit);
    }
};
```

#### 2.3.3 Turn Execution

```
90° Turn Sequence:
  1. Stop forward motion
  2. Set target heading = currentHeading ± 90°
  3. Spin in place: left motor = +PWM, right motor = -PWM (or vice versa)
  4. Monitor gyro yaw continuously
  5. When |yaw - target| < 2°:
       Reduce PWM to crawl speed
  6. When |yaw - target| < 0.5° AND angular rate < 5°/s:
       Stop motors → turn complete
  7. Lock new heading as straight-drive target
```

---

## 3. Power System Design

```
┌──────────┐    ┌──────────────┐    ┌──────────────┐
│ 7.4V     ├───►│ 5V Regulator ├───►│ Arduino Nano │
│ 2S LiPo  │    │ (LM7805 /    │    │ (logic)      │
│ 300 mAh  │    │  buck conv.) │    └──────────────┘
└────┬─────┘    └──────────────┘
     │
     ├───────────────────────────────►┌──────────────┐
     │  (direct to motor driver VCC)  │ TB6612FNG    │
     │                                │ (motor power)│
     └─────┬──────────────────────────┘──────────────┘
           │
           │  ┌────────────────────┐
           └──┤ Voltage Divider    │ (Future: A3)
              │ (battery monitor)  │
              └────────────────────┘
```

| Parameter            | Value                              |
| -------------------- | ---------------------------------- |
| Battery voltage      | 7.4V nominal (6.0V cutoff)         |
| Logic voltage        | 5V regulated                       |
| Motor voltage        | 7.4V direct                        |
| Total current (peak) | ~800 mA (both motors full + logic) |
| Estimated runtime    | ~20 minutes at moderate speed      |

---

## 4. Real-Time Execution Model

```
Main Loop (200 Hz — 5 ms period):

    ┌─────────────────────────────────────────┐
    │  loop() {                               │
    │    handleButton();        // ~50 μs     │
    │                                         │
    │    now = micros();                       │
    │    if (now - prev < 5000) return;        │
    │    dt = (now - prev) / 1e6;              │
    │    prev = now;                           │
    │                                         │
    │    // SENSE                              │
    │    updateYaw(dt);         // ~820 μs    │
    │    walls = readWalls();   // ~360 μs    │
    │    ticks = readEncoders();// ~5 μs      │
    │                                         │
    │    // THINK                              │
    │    stateMachine.update();                │
    │      ├─ if (newCell) {                   │
    │      │    maze.updateWalls();             │
    │      │    navigation.floodFill(); ~2 ms  │
    │      │    nextDir = getNextDir();         │
    │      │  }                                │
    │                                         │
    │    // ACT                                │
    │    correction = headingPID + wallPID;     │
    │    setMotors(base+corr, base-corr);      │
    │  }                                       │
    └─────────────────────────────────────────┘
```

---

## 5. Cell Navigation Logic

```
Cell Transition Detection:

  ticksPerCell = (18 cm) / (wheel circumference / encoder CPR)

  Example: wheel Ø = 30 mm → circumference = 94.2 mm
           gear ratio = 50:1, encoder = 12 CPR
           effective CPR = 600 ticks/rev
           ticks per cell = 180 mm / (94.2 mm / 600) = ~1146 ticks

  avgTicks = (|leftTicks| + |rightTicks|) / 2
  if (avgTicks >= TICKS_PER_CELL):
      → cell boundary reached
      → reset tick counters
      → update position (row, col)
      → read walls
      → re-flood
```

---

## 6. Error Handling & Safety

| Condition                | Detection                                    | Action                             |
| ------------------------ | -------------------------------------------- | ---------------------------------- |
| Heading drift > ±45°     | gyro reading                                 | Immediate stop → `STATE_STOPPED`   |
| Front collision imminent | front IR < 3 cm                              | Immediate stop                     |
| Motor stall              | Encoder ticks = 0 while PWM > 0 for > 500 ms | Reduce speed / re-route            |
| I²C failure              | Wire.endTransmission() ≠ 0                   | Retry 3× then stop                 |
| Loop overrun             | micros() check                               | Log warning, skip non-critical ops |
| Battery low              | A3 voltage divider < 6.0V (future)           | Graceful stop                      |

---

## 7. Coordinate System & Orientation

```
            NORTH (+Y)
               ▲
               │
  WEST ◄───────┼───────► EAST (+X)
               │
               ▼
            SOUTH

  Start: (0, 0) facing NORTH
  Goal:  (7,7), (7,8), (8,7), (8,8)

Robot Orientation Encoding:
  0 = NORTH (facing +Y)
  1 = EAST  (facing +X)
  2 = SOUTH (facing -Y)
  3 = WEST  (facing -X)

Relative Turn Mapping:
  turnRight: orientation = (orientation + 1) % 4
  turnLeft:  orientation = (orientation + 3) % 4
  turnBack:  orientation = (orientation + 2) % 4
```

---

## 8. Serial Debug Protocol

```
Format: CSV @ 115200 baud (disabled in competition mode)

Exploration mode output (once per cell):
  CELL,row,col,walls(hex),flood,heading
  Example: CELL,3,5,0x9,12,90.5

Continuous output (every 50 ms):
  PID,heading,error,correction,leftPWM,rightPWM
  Example: PID,89.7,0.3,4,-122,130

Wall readings (on demand):
  WALL,leftCm,frontCm,rightCm,L_flag,F_flag,R_flag
  Example: WALL,5.2,42.1,6.8,1,0,1
```
