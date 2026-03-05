# ⚙️ Micromouse v2 — Technical Requirements Document

> **Version**: 2.0  
> **Date**: 2026-03-05  
> **Platform**: Arduino Nano (ATmega328P, 16 MHz, 5V)  
> **References**: [PRD.md](file:///home/aditya/Arduino/Micromouse/v2/docs/PRD.md) · [Workflow.md](file:///home/aditya/Arduino/Micromouse/v2/docs/Workflow.md)

---

## 1. System Overview

### 1.1 Block Diagram

```
                             ┌─────────────┐
                             │   3S LiPo   │
                             │   11.1V     │
                             └──────┬──────┘
                                    │
                        ┌───────────┼───────────┐
                        │           │           │
                   ┌────▼────┐ ┌────▼────┐ ┌────▼────┐
                   │ Voltage │ │  5V Reg │ │ TB6612  │
                   │ Divider │ │ (Nano)  │ │  VM     │
                   └────┬────┘ └────┬────┘ └────┬────┘
                        │          │           │
                   ┌────▼──────────▼───────────▼────┐
                   │         ARDUINO NANO            │
                   │                                 │
                   │  D2,D7 ←── Left Encoder         │
                   │  D3,D8 ←── Right Encoder        │
                   │  D5,D11,D12 ──→ Right Motor     │
                   │  D6,D9,D10  ──→ Left Motor      │
                   │  D13 ──→ STBY                    │
                   │  D4 ←── Push Button              │
                   │  A4,A5 ←→ MPU6050 (I²C)         │
                   │  A2 ──► Shared US Trigger       │
                   │  A3 ←── Front US Echo           │
                   │  A0 ←── Left US Echo            │
                   │  A1 ←── Right US Echo           │
                   └────────────────────────────────┘
```

### 1.2 Operating Parameters

| Parameter         | Specification                                |
| ----------------- | -------------------------------------------- |
| Supply Voltage    | 11.1V (3S LiPo, nominal)                     |
| Logic Voltage     | 5V (Nano onboard regulator)                  |
| Motor Voltage     | 6–12V (via TB6612 VM)                        |
| Clock Speed       | 16 MHz                                       |
| Flash Memory      | 32 KB (minus 2 KB bootloader = 30 KB usable) |
| SRAM              | 2048 bytes                                   |
| EEPROM            | 1024 bytes                                   |
| Control Loop Rate | 200 Hz (5 ms interval)                       |
| Max PWM Frequency | ~490 Hz (D5, D6), ~976 Hz (D9, D10, D11)     |

---

## 2. Hardware Interface Specifications

### 2.1 Motor Driver — TB6612FNG

#### Pin Assignment

| Function    | TB6612 Pin | Arduino Pin | Type           |
| ----------- | ---------- | ----------- | -------------- |
| Left PWM    | PWMA       | D5          | PWM Output     |
| Left Dir 1  | AIN1       | D12         | Digital Output |
| Left Dir 2  | AIN2       | D11         | Digital Output |
| Right PWM   | PWMB       | D6          | PWM Output     |
| Right Dir 1 | BIN1       | D10         | Digital Output |
| Right Dir 2 | BIN2       | D9          | Digital Output |
| Standby     | STBY       | D13         | Digital Output |

#### Motor Control Truth Table

| AIN1 | AIN2 | PWMA  | Motor Action          |
| ---- | ---- | ----- | --------------------- |
| HIGH | LOW  | 0–255 | Forward (speed = PWM) |
| LOW  | HIGH | 0–255 | Reverse (speed = PWM) |
| LOW  | LOW  | X     | Brake (active stop)   |
| HIGH | HIGH | X     | Brake (active stop)   |

#### Driver Specifications

| Parameter              | Value            |
| ---------------------- | ---------------- |
| Max continuous current | 1.2A per channel |
| Peak current           | 3.2A per channel |
| VM range               | 4.5V – 13.5V     |
| Logic voltage (VCC)    | 2.7V – 5.5V      |
| Standby current        | < 1 µA           |
| PWM frequency limit    | Up to 100 kHz    |

#### Implementation Requirements

```cpp
void setMotors(int leftPWM, int rightPWM);
// leftPWM, rightPWM: -255 to +255
// Negative = reverse, 0 = brake, Positive = forward
// Must set STBY HIGH before use

void brake();
// Set AIN1=LOW, AIN2=LOW, BIN1=LOW, BIN2=LOW
// Active braking — stops motors immediately
```

---

### 2.2 Quadrature Encoders

#### Pin Assignment

| Signal              | Arduino Pin | Interrupt | Notes                     |
| ------------------- | ----------- | --------- | ------------------------- |
| Left A (Channel A)  | D2          | INT0      | RISING edge ISR           |
| Left B (Channel B)  | D7          | —         | Direction read inside ISR |
| Right A (Channel A) | D3          | INT1      | RISING edge ISR           |
| Right B (Channel B) | D8          | —         | Direction read inside ISR |

#### Encoder Specifications

| Parameter                   | Value                   |
| --------------------------- | ----------------------- |
| Counts Per Revolution (CPR) | 355                     |
| Wheel Diameter              | 34 mm                   |
| Wheel Circumference         | 106.81 mm               |
| Distance per Tick           | 0.3009 mm               |
| Ticks per Cell (160 mm)     | 532 ticks               |
| Max detectable RPM @ 16 MHz | ~2700 RPM (theoretical) |

#### ISR Implementation

```cpp
volatile long leftTicks = 0;
volatile long rightTicks = 0;

void leftEncoderISR() {
  // Channel B determines direction
  if (digitalRead(LEFT_B_PIN) == HIGH)
    leftTicks++;
  else
    leftTicks--;
}

void rightEncoderISR() {
  if (digitalRead(RIGHT_B_PIN) == HIGH)
    rightTicks++;
  else
    rightTicks--;
}

// In setup():
attachInterrupt(digitalPinToInterrupt(2), leftEncoderISR, RISING);
attachInterrupt(digitalPinToInterrupt(3), rightEncoderISR, RISING);
```

#### Critical Notes

- ISR must be minimal — no `Serial.print()`, no floating point, no `delay()`
- `leftTicks` and `rightTicks` must be declared `volatile`
- Read ticks atomically: disable interrupts → copy → re-enable
  ```cpp
  noInterrupts();
  long l = leftTicks;
  long r = rightTicks;
  interrupts();
  ```

---

### 2.3 MPU6050 IMU

#### Connection

| MPU6050 Pin | Arduino Pin          |
| ----------- | -------------------- |
| VCC         | 5V                   |
| GND         | GND                  |
| SDA         | A4                   |
| SCL         | A5                   |
| INT         | Not connected        |
| AD0         | GND (address = 0x68) |

#### Specifications Used

| Parameter            | Value                               |
| -------------------- | ----------------------------------- |
| I²C Address          | 0x68                                |
| Gyroscope Range      | ±250 °/s (sensitivity: 131 LSB/°/s) |
| Accelerometer Range  | ±2g (not used for navigation)       |
| Sample Rate          | 200 Hz (matches control loop)       |
| Typical Z-axis Drift | 1–5 °/min (varies per unit)         |

#### Yaw Angle Integration

```cpp
// Read raw Z-axis gyroscope
int16_t rawZ = readGyroZ();

// Convert to °/s (after bias subtraction)
float gyroZ = (rawZ - gyroBias) / 131.0;

// Integrate to get angle
float dt = (micros() - lastTime) / 1000000.0;
yawAngle += gyroZ * dt;

// Wrap to ±180°
if (yawAngle > 180.0) yawAngle -= 360.0;
if (yawAngle < -180.0) yawAngle += 360.0;
```

#### Calibration Procedure

```
1. Robot MUST be stationary
2. Collect 500 raw Z-axis samples (~2.5s at 200 Hz)
3. Calculate mean → gyroBias
4. Calculate variance
5. If variance > threshold → reject (robot was bumped)
6. Store gyroBias for runtime use
```

```cpp
float sumZ = 0, sumZSq = 0;
for (int i = 0; i < 500; i++) {
  int16_t raw = readGyroZ();
  sumZ += raw;
  sumZSq += (float)raw * raw;
  delay(5);
}
float mean = sumZ / 500.0;
float variance = (sumZSq / 500.0) - (mean * mean);

if (variance > 100.0) {
  // Calibration failed — vibration detected
  return false;
}
gyroBias = mean;
```

#### Drift Mitigation Strategy

| Scenario         | Strategy                                                     |
| ---------------- | ------------------------------------------------------------ |
| Driving straight | Use encoders for distance, MPU only for correction reference |
| Turning          | Use MPU as primary angle source                              |
| Between walls    | Apply wall-centering sensor correction to nudge yaw          |
| Stopped          | Do NOT integrate (gyro drifts even at standstill)            |

---

### 2.4 Ultrasonic Sensors — HC-SR04

#### Pin Assignment

| Sensor | Trig Pin         | Echo Pin | Orientation                |
| ------ | ---------------- | -------- | -------------------------- |
| Front  | A2 (shared trig) | A3       | 0° (forward)               |
| Left   | A2 (shared trig) | A0       | 90° (perpendicular left)   |
| Right  | A2 (shared trig) | A1       | -90° (perpendicular right) |

> All 3 sensor trig pins are wired together to A2. Sensors must be read **sequentially** with ~30ms delay between reads to avoid echo cross-talk.

#### HC-SR04 Specifications

| Parameter         | Value                               |
| ----------------- | ----------------------------------- |
| Operating Voltage | 5V                                  |
| Trigger Pulse     | 10 µs HIGH                          |
| Echo Output       | HIGH pulse proportional to distance |
| Range             | 2 cm – 400 cm                       |
| Resolution        | ~3 mm                               |
| Measurement Cycle | ~30 ms (includes 60 ms timeout)     |
| Beam Angle        | ~15°                                |

#### Distance Calculation

```cpp
float readDistance_cm(int trigPin, int echoPin) {
  // Trigger pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read echo duration
  long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout

  if (duration == 0) return 999.0; // No echo = no wall

  // Speed of sound: 343 m/s = 0.0343 cm/µs
  // Distance = duration × 0.0343 / 2
  return duration * 0.01715;
}
```

#### Averaging Filter

```cpp
float readDistanceFiltered(int trigPin, int echoPin) {
  float sum = 0;
  int validCount = 0;

  for (int i = 0; i < 5; i++) {
    float d = readDistance_cm(trigPin, echoPin);
    if (d < 400.0) { // Ignore timeouts
      sum += d;
      validCount++;
    }
    delayMicroseconds(200); // Brief cooldown between pings
  }

  if (validCount == 0) return 999.0;
  return sum / validCount;
}
```

#### Wall Detection Thresholds

| State                         | Distance | Constant                |
| ----------------------------- | -------- | ----------------------- |
| Wall present                  | < 10 cm  | `WALL_DETECT_THRESHOLD` |
| No wall                       | > 14 cm  | `WALL_CLEAR_THRESHOLD`  |
| Too close (collision risk)    | < 5 cm   | `WALL_DANGER_THRESHOLD` |
| Centering target (side walls) | 7–8 cm   | `WALL_CENTER_TARGET`    |

#### Timing Budget

```
3 sensors × 30 ms each = ~90 ms per full scan
At 200 Hz control loop (5 ms), sensors are the bottleneck.

Strategy: Read ONE sensor per control loop iteration.
  Loop 0: Read front
  Loop 1: Read left
  Loop 2: Read right
  Loop 3–5: No read (use cached values)

Result: Sensor data refreshes every ~30 ms (33 Hz) — acceptable.
```

#### Cross-Talk Prevention

```
NEVER trigger two ultrasonic sensors simultaneously.
Sequential reading with minimum 2 ms gap between sensors.
Physical shielding (foam/tape) between adjacent sensor cones helps.
```

---

### 2.5 Power Regulation — L7805CV

#### Circuit

```
3S LiPo (11.1V)
    │
    ├──► TB6612 VM (motor power)
    │
    └──► L7805CV Linear Voltage Regulator
              │
              OUT ──► 5V (to Arduino, sensors, logic)
              GND ──► GND

    Not connected to any Arduino pin.
```

#### L7805CV Specifications

| Parameter          | Value          |
| ------------------ | -------------- |
| Input Voltage      | 7V – 35V       |
| Output Voltage     | 5V (regulated) |
| Max Output Current | 1.5A           |
| Dropout Voltage    | ~2V            |

> **Note**: No battery voltage monitoring is implemented. Ensure battery is charged before each run.

---

### 2.6 Push Button (Start/Stop)

#### Pin Assignment

| Signal | Arduino Pin | Mode         | Notes                       |
| ------ | ----------- | ------------ | --------------------------- |
| Button | D4          | INPUT_PULLUP | Press = LOW, Release = HIGH |

#### Debounce Implementation

```cpp
#define BUTTON_PIN 4
#define DEBOUNCE_MS 50

static bool lastButtonState = HIGH;
static unsigned long lastDebounceTime = 0;

void handleButton() {
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
    static bool currentState = HIGH;
    if (reading != currentState) {
      currentState = reading;
      if (currentState == LOW) {
        // Button pressed — toggle state
        onButtonPress();
      }
    }
  }

  lastButtonState = reading;
}

void onButtonPress() {
  switch (botState) {
    case STATE_IDLE:
      botState = STATE_CALIBRATING;
      break;
    case STATE_EXPLORING:
    case STATE_MOVING:
    case STATE_TURNING:
    case STATE_SPEED_RUN:
      botState = STATE_STOPPED; // Emergency stop
      brake();
      break;
    case STATE_STOPPED:
      botState = STATE_IDLE; // Reset
      break;
  }
}
```

#### Wiring

```
D4 ──── Button ──── GND
(no external resistor needed — uses internal pullup)
```

---

## 3. Control System Specifications

### 3.1 Straight-Line PID Controller

#### Purpose

Maintain equal encoder counts between left and right wheels during forward movement.

#### Control Law

```
error(t) = leftTicks - rightTicks

integral += error × dt
integral = constrain(integral, -MAX_INTEGRAL, MAX_INTEGRAL)

derivative = (error - prevError) / dt

correction = Kp × error + Ki × integral + Kd × derivative

leftMotor  = BASE_SPEED - correction
rightMotor = BASE_SPEED + correction
```

#### Parameters

| Parameter    | Starting Value | Tuning Range | Units        |
| ------------ | :------------: | :----------: | ------------ |
| Kp           |      2.0       |  0.5 – 8.0   | PWM/tick     |
| Ki           |      0.0       |  0.0 – 0.1   | PWM/(tick·s) |
| Kd           |      1.0       |  0.0 – 5.0   | PWM·s/tick   |
| BASE_SPEED   |      100       |   60 – 200   | PWM (0–255)  |
| MAX_INTEGRAL |      1000      |      —       | ticks·s      |

#### Loop Timing

```
Control interval: 5 ms (200 Hz)
dt = 0.005 s

if (micros() - lastLoopTime < 5000) return;  // Non-blocking
```

---

### 3.2 Turn PID Controller

#### Purpose

Rotate robot to a target yaw angle using MPU6050 feedback.

#### Control Law (P-only sufficient for turns)

```
error = targetAngle - currentYaw
// Wrap error to ±180°
if (error > 180) error -= 360;
if (error < -180) error += 360;

turnSpeed = Kp_turn × error
turnSpeed = constrain(turnSpeed, -MAX_TURN_SPEED, MAX_TURN_SPEED)

leftMotor  = -turnSpeed   // Pivot: opposite directions
rightMotor = +turnSpeed
```

#### Parameters

| Parameter       | Value  | Notes                                  |
| --------------- | :----: | -------------------------------------- | ----- | ------ |
| Kp_turn         |  4.0   | Higher than straight-line Kp           |
| MAX_TURN_SPEED  |  150   | PWM clamp                              |
| ANGLE_TOLERANCE |  2.0°  | Stop turning when                      | error | < this |
| SETTLE_TIME     | 200 ms | Wait after motor stop to verify angle  |
| MIN_TURN_SPEED  |   40   | Below this, motors stall — clamp above |

#### Turn Completion Detection

```cpp
bool turnComplete(float targetAngle) {
  float error = targetAngle - yawAngle;
  wrapAngle(error);

  if (abs(error) < ANGLE_TOLERANCE) {
    brake();
    delay(SETTLE_TIME);

    // Re-read angle after settling
    updateAngle();
    error = targetAngle - yawAngle;
    wrapAngle(error);

    return (abs(error) < ANGLE_TOLERANCE * 2);
  }
  return false;
}
```

---

### 3.3 Wall-Centering Correction (Optional Enhancement)

When both side walls are present, use sensor differential to center robot:

```
sideError = leftDistance - rightDistance
wallCorrection = Kp_wall × sideError

// Blend with encoder PID
totalCorrection = encoderCorrection + wallCorrection
```

| Parameter        |               Value                |
| ---------------- | :--------------------------------: |
| Kp_wall          |                0.5                 |
| Only active when | Both left AND right walls detected |

---

## 4. Maze & Algorithm Specifications

### 4.1 Maze Representation

#### Memory Layout

```cpp
#define MAZE_SIZE 8

struct Cell {
  uint8_t walls;     // Bits: [3]=West [2]=South [1]=East [0]=North
  uint8_t distance;  // 0–255, 255 = unvisited/unreachable
  bool visited;      // Has the robot physically entered this cell?
};

Cell maze[MAZE_SIZE][MAZE_SIZE];  // 192 bytes
```

#### Wall Bitmask Operations

```cpp
#define WALL_N  0x01  // bit 0
#define WALL_E  0x02  // bit 1
#define WALL_S  0x04  // bit 2
#define WALL_W  0x08  // bit 3

// Set a wall
maze[x][y].walls |= WALL_N;

// Check a wall
bool hasNorthWall = (maze[x][y].walls & WALL_N) != 0;

// Clear a wall
maze[x][y].walls &= ~WALL_N;
```

#### Coordinate System

```
Y
^
|  (0,7) (1,7) ... (7,7)
|  (0,6) (1,6) ... (7,6)
|   ...   ...       ...
|  (0,1) (1,1) ... (7,1)
|  (0,0) (1,0) ... (7,0)  ← START
+-------------------------→ X
```

- **Start**: (0, 0), facing NORTH
- **Goal**: (3,3), (3,4), (4,3), (4,4) — center 4 cells

#### Sensor-to-Wall Direction Mapping

The robot's sensors are body-relative but walls are stored in absolute directions.

```cpp
// Given robot heading, map sensor readings to absolute walls
Direction frontDir = heading;                    // Same as heading
Direction leftDir  = (heading + 3) % 4;          // 90° counterclockwise
Direction rightDir = (heading + 1) % 4;          // 90° clockwise
Direction backDir  = (heading + 2) % 4;          // 180°

// Opposite wall mapping (for neighbor cell)
Direction opposite(Direction d) {
  return (Direction)((d + 2) % 4);
}
```

#### Wall Update Algorithm

```cpp
void updateWalls(uint8_t x, uint8_t y, Direction heading,
                 bool wF, bool wL, bool wR) {

  Direction front = heading;
  Direction left  = (Direction)((heading + 3) % 4);
  Direction right = (Direction)((heading + 1) % 4);

  // Set walls on current cell
  if (wF) maze[x][y].walls |= (1 << front);
  if (wL) maze[x][y].walls |= (1 << left);
  if (wR) maze[x][y].walls |= (1 << right);

  // Mirror walls onto neighbor cells
  if (wF) {
    int nx, ny;
    getNeighbor(x, y, front, &nx, &ny);
    if (inBounds(nx, ny))
      maze[nx][ny].walls |= (1 << opposite(front));
  }
  // ... same for left and right
}
```

---

### 4.2 Flood Fill Algorithm

#### BFS Queue Implementation (Circular Buffer)

SRAM-efficient circular queue without dynamic allocation:

```cpp
#define QUEUE_SIZE 64

struct Queue {
  uint8_t data[QUEUE_SIZE][2]; // [x, y] pairs
  uint8_t head;
  uint8_t tail;
  uint8_t count;
};

void enqueue(Queue* q, uint8_t x, uint8_t y) {
  if (q->count >= QUEUE_SIZE) return; // Overflow protection
  q->data[q->tail][0] = x;
  q->data[q->tail][1] = y;
  q->tail = (q->tail + 1) % QUEUE_SIZE;
  q->count++;
}

bool dequeue(Queue* q, uint8_t* x, uint8_t* y) {
  if (q->count == 0) return false;
  *x = q->data[q->head][0];
  *y = q->data[q->head][1];
  q->head = (q->head + 1) % QUEUE_SIZE;
  q->count--;
  return true;
}
```

#### Flood Fill Function

```cpp
void floodFill() {
  Queue q;
  q.head = q.tail = q.count = 0;

  // Reset all distances
  for (uint8_t x = 0; x < MAZE_SIZE; x++)
    for (uint8_t y = 0; y < MAZE_SIZE; y++)
      maze[x][y].distance = 255;

  // Seed goal cells with distance 0
  uint8_t goals[][2] = {{3,3},{3,4},{4,3},{4,4}};
  for (int i = 0; i < 4; i++) {
    maze[goals[i][0]][goals[i][1]].distance = 0;
    enqueue(&q, goals[i][0], goals[i][1]);
  }

  // BFS expansion
  uint8_t cx, cy;
  while (dequeue(&q, &cx, &cy)) {
    uint8_t newDist = maze[cx][cy].distance + 1;

    // Check all 4 neighbors
    int8_t dx[] = {0, 1, 0, -1}; // N, E, S, W
    int8_t dy[] = {1, 0, -1, 0};

    for (int d = 0; d < 4; d++) {
      // Skip if wall exists in this direction
      if (maze[cx][cy].walls & (1 << d)) continue;

      int8_t nx = cx + dx[d];
      int8_t ny = cy + dy[d];

      // Bounds check
      if (nx < 0 || nx >= MAZE_SIZE || ny < 0 || ny >= MAZE_SIZE) continue;

      // Update if shorter path found
      if (maze[nx][ny].distance > newDist) {
        maze[nx][ny].distance = newDist;
        enqueue(&q, nx, ny);
      }
    }
  }
}
```

#### Time Complexity

| Maze Size | Cells | Worst-case BFS iterations | Estimated Time (16 MHz) |
| --------- | ----- | ------------------------- | ----------------------- |
| 8×8       | 64    | ~256                      | < 2 ms                  |
| 16×16     | 256   | ~1024                     | < 8 ms                  |

Flood fill runs well within a single 5 ms control loop.

---

### 4.3 Navigation Decision Logic

```cpp
Direction getNextDirection(uint8_t x, uint8_t y) {
  uint8_t minDist = 255;
  Direction bestDir = NORTH;

  int8_t dx[] = {0, 1, 0, -1};
  int8_t dy[] = {1, 0, -1, 0};

  for (int d = 0; d < 4; d++) {
    // Skip if wall blocks this direction
    if (maze[x][y].walls & (1 << d)) continue;

    int8_t nx = x + dx[d];
    int8_t ny = y + dy[d];

    if (nx < 0 || nx >= MAZE_SIZE || ny < 0 || ny >= MAZE_SIZE) continue;

    if (maze[nx][ny].distance < minDist) {
      minDist = maze[nx][ny].distance;
      bestDir = (Direction)d;
    }
  }

  return bestDir;
}
```

#### Turn Decision

```cpp
void turnToFace(Direction target) {
  int turns = (target - heading + 4) % 4;

  switch (turns) {
    case 0: break;               // Already facing correct direction
    case 1: turnRight90(); break;
    case 2: turnBack();   break;
    case 3: turnLeft90();  break;
  }

  heading = target;
}
```

---

### 4.4 Speed Run Path Storage

```cpp
#define MAX_PATH 64

char speedRunPath[MAX_PATH];  // 'F', 'L', 'R'
uint8_t pathLength = 0;

void computeSpeedRunPath() {
  pathLength = 0;
  uint8_t cx = 0, cy = 0;
  Direction cdir = NORTH;

  while (!isGoal(cx, cy) && pathLength < MAX_PATH) {
    Direction next = getNextDirection(cx, cy);
    int turns = (next - cdir + 4) % 4;

    switch (turns) {
      case 0: speedRunPath[pathLength++] = 'F'; break;
      case 1: speedRunPath[pathLength++] = 'R';
              speedRunPath[pathLength++] = 'F'; break;
      case 2: speedRunPath[pathLength++] = 'B';
              speedRunPath[pathLength++] = 'F'; break;
      case 3: speedRunPath[pathLength++] = 'L';
              speedRunPath[pathLength++] = 'F'; break;
    }

    // Advance position
    int8_t dx[] = {0, 1, 0, -1};
    int8_t dy[] = {1, 0, -1, 0};
    cx += dx[next];
    cy += dy[next];
    cdir = next;
  }
}
```

---

## 5. Timing & Execution Model

### 5.1 Main Control Loop

```cpp
// Target: 200 Hz non-blocking loop
#define LOOP_INTERVAL_US 5000

unsigned long prevLoopTime = 0;
uint8_t sensorSlot = 0;

void loop() {
  handleButton();  // Debounced start/stop (every call)

  unsigned long now = micros();
  if (now - prevLoopTime < LOOP_INTERVAL_US) return;
  prevLoopTime = now;

  // IMU update (every loop — fast)
  updateYawAngle();

  // Sensor reading (round-robin — one per loop)
  switch (sensorSlot) {
    case 0: cachedFront = readDistanceFiltered(FRONT_TRIG, FRONT_ECHO); break;
    case 1: cachedLeft  = readDistanceFiltered(LEFT_TRIG, LEFT_ECHO);   break;
    case 2: cachedRight = readDistanceFiltered(RIGHT_TRIG, RIGHT_ECHO); break;
  }
  sensorSlot = (sensorSlot + 1) % 3;

  // State machine
  switch (botState) {
    case STATE_IDLE:        break;
    case STATE_CALIBRATING: runCalibration(); break;
    case STATE_EXPLORING:   runExplorationStep(); break;
    case STATE_TURNING:     runTurnStep(); break;
    case STATE_MOVING:      runMoveStep(); break;
    case STATE_SPEED_RUN:   runSpeedRunStep(); break;
    case STATE_STOPPED:     brake(); break;
  }
}
```

### 5.2 Timing Budget per Loop Iteration (5 ms)

| Task                  | Time        | Notes                                |
| --------------------- | ----------- | ------------------------------------ |
| IMU read (I²C)        | ~0.7 ms     | 400 kHz I²C                          |
| Ultrasonic (1 sensor) | ~0.05 ms    | Uses cached result, trigger is async |
| Encoder read          | ~0.01 ms    | Just copy volatile vars              |
| PID computation       | ~0.1 ms     | Float math                           |
| Motor output          | ~0.05 ms    | `analogWrite()` calls                |
| Decision logic        | ~0.2 ms     | Only runs at cell boundaries         |
| Flood fill            | ~2 ms       | Only runs at cell boundaries         |
| **Total (moving)**    | **~1.1 ms** | Well within 5 ms budget              |
| **Total (at cell)**   | **~3.1 ms** | With flood fill, still within budget |

---

## 6. EEPROM Usage (Optional)

Store maze state across power cycles for speed run resumption.

| Address Range | Data                              | Size                  |
| ------------- | --------------------------------- | --------------------- |
| 0x000 – 0x001 | Magic bytes (0xAA, 0x55)          | 2 bytes               |
| 0x002         | Maze version / run count          | 1 byte                |
| 0x003 – 0x042 | Wall data (64 cells × 1 byte)     | 64 bytes              |
| 0x043 – 0x082 | Distance data (64 cells × 1 byte) | 64 bytes              |
| 0x083 – 0x0C2 | Speed run path                    | 64 bytes              |
| 0x0C3         | Path length                       | 1 byte                |
| **Total**     |                                   | **196 bytes** of 1024 |

```cpp
void saveMaze() {
  EEPROM.write(0, 0xAA);
  EEPROM.write(1, 0x55);
  for (int i = 0; i < 64; i++) {
    EEPROM.write(3 + i, maze[i/8][i%8].walls);
    EEPROM.write(67 + i, maze[i/8][i%8].distance);
  }
}

bool loadMaze() {
  if (EEPROM.read(0) != 0xAA || EEPROM.read(1) != 0x55)
    return false; // No saved maze
  // ... restore
  return true;
}
```

---

## 7. Serial Debug Protocol

### 7.1 Debug Output Format

```
[T=12345] S:EXPLORING P:(3,2) H:NORTH D:5
[T=12345] W: F=1 L=0 R=1  US: F=8.2 L=22.1 R=6.4
[T=12345] PID: e=3 c=6.0  M: L=94 R=106
[T=12345] ENC: L=1598 R=1601
```

| Prefix | Meaning                             |
| ------ | ----------------------------------- |
| `S:`   | Current state                       |
| `P:`   | Position (x, y)                     |
| `H:`   | Heading                             |
| `D:`   | Flood fill distance of current cell |
| `W:`   | Wall booleans                       |
| `US:`  | Raw ultrasonic distances (cm)       |
| `PID:` | Error and correction values         |
| `M:`   | Motor PWM values                    |
| `ENC:` | Encoder tick counts                 |

### 7.2 Serial Commands (Input)

| Command | Action            |
| ------- | ----------------- |
| `s`     | Start exploration |
| `x`     | Emergency stop    |
| `p`     | Print full maze   |
| `r`     | Start speed run   |
| `c`     | Re-calibrate MPU  |
| `d`     | Dump debug state  |

### 7.3 Baud Rate

```cpp
Serial.begin(115200);  // Fast enough for debug without slowing loop
```

> ⚠️ Use `F()` macro for all string literals: `Serial.println(F("text"))` — saves SRAM.

---

## 8. Error Handling

| Error Condition         | Detection                                 | Response                                  |
| ----------------------- | ----------------------------------------- | ----------------------------------------- |
| MPU calibration failure | Variance > threshold                      | Retry 3 times, then enter `STATE_STOPPED` |
| Encoder stall           | No ticks for > 500 ms while motors active | Stop, log error, enter `STATE_STOPPED`    |
| Battery critical        | V < 9.9V                                  | Immediate stop, save maze to EEPROM       |
| Ultrasonic timeout      | `pulseIn` returns 0                       | Return 999 cm (no wall), continue         |
| Flood fill: no path     | All neighbors distance = 255              | Turn around, re-explore                   |
| SRAM overflow           | Stack corruption (hard to detect)         | Use `freeMemory()` check at startup       |
| I²C hang                | MPU6050 stops responding                  | Wire.end() + Wire.begin() reset           |

#### Free Memory Check

```cpp
extern int __heap_start, *__brkval;
int freeMemory() {
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

// In setup():
Serial.print(F("Free RAM: "));
Serial.println(freeMemory());
// Should be > 500 bytes
```

---

## 9. Build & Upload Configuration

### 9.1 Arduino IDE Settings

| Setting   | Value                       |
| --------- | --------------------------- |
| Board     | Arduino Nano                |
| Processor | ATmega328P (Old Bootloader) |
| Port      | /dev/ttyUSB0 (Linux)        |
| Baud      | 115200                      |

### 9.2 Required Libraries

| Library | Version  | Purpose                     |
| ------- | -------- | --------------------------- |
| Wire    | Built-in | I²C communication (MPU6050) |
| EEPROM  | Built-in | Maze persistence            |
| —       | —        | No external dependencies    |

### 9.3 Multi-File Build (Arduino IDE)

Arduino IDE compiles all `.ino`, `.cpp`, and `.h` files in the project folder. Critical rules:

- **Main sketch** must be named to match the folder: `v2.ino` (or rename folder)
- **Struct definitions** used across files must be in their own `.h` header
- **`#pragma once`** at the top of every `.h` file
- **Avoid forward declarations** — Arduino's auto-prototype generation can break with custom structs

```
v2/
├── v2.ino         ← Main sketch (setup + loop)
├── config.h       ← All constants and pin definitions
├── motors.h/.cpp
├── encoders.h/.cpp
├── imu.h/.cpp
├── sensors.h/.cpp
├── pid.h/.cpp
├── movement.h/.cpp
├── maze.h/.cpp
├── floodfill.h/.cpp
└── navigation.h/.cpp
```

---

## 10. Complete Constants Reference (`config.h`)

```cpp
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
// Right sensor: reassign from A6/A7 — see TRD §2.4

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

// ─── Control Loop ──────────────────────────────
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
```
