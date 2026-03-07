# 🧭 Complete Workflow for Building a Micromouse (Flood Fill)

Your robot stack:

- **Arduino Nano**
- **Encoder motors**
- **Ultrasonic sensors**
- **TB6612 motor driver**
- **MPU6050**
- **Voltage divider (battery monitoring)**

We will build it in **7 stages**.

---

# Stage 1 — Hardware Setup (Foundation)

Before coding algorithms, your robot must **move reliably**.

### Step 1: Connect Components

Typical connection layout:

**Motor Driver → Arduino**

| Motor Driver | Arduino |
| ------------ | ------- | ----------- |
| PWMA         | D5      | Left speed  |
| AIN1         | D12     | Left dir    |
| AIN2         | D11     | Left dir    |
| PWMB         | D6      | Right speed |
| BIN1         | D10     | Right dir   |
| BIN2         | D9      | Right dir   |
| STBY         | D13     |

**Encoders**

| Encoder | Arduino |
| ------- | ------- |
| Left A  | D2      |
| Left B  | D7      |
| Right A | D3      |
| Right B | D8      |

**MPU6050**

| MPU | Arduino |
| --- | ------- |
| VCC | 5V      |
| GND | GND     |
| SDA | A4      |
| SCL | A5      |

**Ultrasonic Sensors**

| Sensor | Trig        | Echo |
| ------ | ----------- | ---- |
| All 3  | A2 (shared) | —    |
| Front  | —           | A3   |
| Left   | —           | A0   |
| Right  | —           | A1   |

**Power Regulation (L7805CV)**

Battery → L7805CV → 5V output (not connected to any Arduino pin)

**Push Button**

| Button | Arduino |
| ------ | ------- |
| Pin 1  | D4      |
| Pin 2  | GND     |

Uses `INPUT_PULLUP` — no external resistor needed.

---

### Step 2: Test Each Component Individually

Create **separate test programs**

Test in this order:

1️⃣ Motor forward/backward
2️⃣ Encoder count
3️⃣ MPU yaw angle
4️⃣ Ultrasonic distance
5️⃣ Push button start/stop

⚠️ Do NOT combine everything yet.

---

# Stage 2 — Motion Control (Robot Movement)

Now your robot must move **accurately by distance and angle**.

---

### Step 3: Encoder Distance Control

Goal:

```
moveForward(distance_cm)
```

Use encoder counts.

```
1 cell = 16 cm
Wheel diameter = 34 mm
Encoder CPR = 355
Distance per tick = 0.30 mm
Ticks per cell ≈ 533
```

Robot must move **exactly one cell**.

---

### Step 4: Accurate Turning

Implement:

```
turnLeft90()
turnRight90()
turnBack()
```

Use:

**MPU6050 yaw angle**

Example:

```
while(yaw < targetAngle)
```

---

### Step 5: Straight Line Correction

Use encoders to keep robot straight.

```
error = leftEncoder - rightEncoder
motorCorrection = Kp * error
```

---

# Stage 3 — Wall Detection

Now robot must detect walls.

---

### Step 6: Ultrasonic Wall Detection

Detect:

```
front wall
left wall
right wall
```

Example logic:

```
if(frontDistance < 10cm)
    wallFront = true
```

Store in:

```
bool wallLeft
bool wallRight
bool wallFront
```

---

# Stage 4 — Maze Representation

Now you build the **virtual maze in memory**.

Micromouse maze:

```
8 x 8 grid
```

Create arrays:

```
Cell maze[8][8]
```

Each cell stores:

```
uint8_t walls     // bitmask: N E S W
uint8_t distance  // flood fill value
bool visited
```

Direction indexing:

```
0 = North
1 = East
2 = South
3 = West
```

---

### Step 7: Store Walls

When robot enters a cell:

```
read sensors
update maze walls
```

Example:

```
if(frontWall)
    walls[x][y] |= (1 << dir)
```

Also update the **neighbor's matching wall**.

---

# Stage 5 — Flood Fill Algorithm

This is the **brain**.

Goal:

Robot always moves to **lowest distance cell**.

---

### Step 8: Initialize Maze

Goal cells (center of 8×8):

```
(3,3)
(3,4)
(4,3)
(4,4)
```

Set:

```
distance = 0
```

Run flood fill to fill rest.

---

### Step 9: Flood Fill

Algorithm:

```
distance = 0 for goal

for each neighbor (no wall between)
    distance = parent + 1
```

Result looks like:

```
7  6  5  4  4  5  6  7
6  5  4  3  3  4  5  6
5  4  3  2  2  3  4  5
4  3  2  1  1  2  3  4
4  3  2  1  1  2  3  4
5  4  3  2  2  3  4  5
6  5  4  3  3  4  5  6
7  6  5  4  4  5  6  7
```

---

### Step 10: Choose Next Cell

Robot always goes to:

```
neighbor with smallest value
```

Example:

```
if(front < left and front < right)
    moveForward()
```

---

# Stage 6 — Exploration Mode

Now robot **explores unknown maze**.

Each step in `exploreStep()`:

```
1. Settle 150ms (let vibration stop)
2. Read 3 samples per sensor, majority vote (2/3)
3. updateWalls(x, y, heading, wFront, wLeft, wRight)
4. maze[x][y].visited = true
5. if isGoal(x, y): STOP
6. floodFill()  ← recalculate all 64 distances with new walls
7. nextDir = getBestDirection(x, y, heading)
8. turnToFace(nextDir, heading)
9. moveForward(CELL_SIZE_MM / 10.0)
10. x, y = x + dx[nextDir], y + dy[nextDir]
```

Turning logic in `turnToFace(target, current)`:

```
diff = (target - current + 4) % 4
0 → no turn
1 → turnRight90()
2 → turnBack()
3 → turnLeft90()
```

Key Bugs to Avoid:

```
- WALL_DETECT_CM too large:  use MAZE_WALL_CM = 7.0 for recording
- BFS queue too small:        use QUEUE_SIZE = 128
- Sensor cross-talk:          use 15ms delay between pings
```

Repeat until:

```
isGoal(x, y) == true
```

---

# Stage 7 — Speed Run Mode

After exploration the maze map is fully known.

Robot now knows shortest path.

### Step 11: Trace Optimal Path

From the FULLY EXPLORED maze, trace backwards from start `(0,0)` to goal:

```
path = []
current = (0, 0)
while not isGoal(current):
  next = getBestDirection(current, heading)
  path.append(next)
  current = move(current, next)
```

Encode each move as a character:

```
'F' = keep going straight (no turn)
'R' = turn right then forward
'L' = turn left then forward
'B' = turn back (180) then forward
```

So the path array looks like:

```
FFLFRFF
```

Store in `speedRunPath[MAX_PATH]` as char array.

### Step 12: Speed Run Replay

Now replay the stored path at `SPEED_RUN_SPEED`:

```
for each command in speedRunPath:
  if 'F': moveForwardFast(1 cell)
  if 'R': turnRight90(); moveForwardFast(1 cell)
  if 'L': turnLeft90();  moveForwardFast(1 cell)
  if 'B': turnBack();    moveForwardFast(1 cell)
```

Key: `SPEED_RUN_SPEED` should be higher than `BASE_SPEED`.
Start with `BASE_SPEED + 30`, then tune up.

Now run **fast run**.

---

# 🧠 Complete Program Architecture

```
v2/
├── v2.ino                    # Entry point + state machine
├── platformio.ini
│
├── src/
│   ├── config.h              # Pins, constants, enums
│   ├── motors.h / .cpp
│   ├── encoders.h / .cpp
│   ├── imu.h / .cpp
│   ├── sensors.h / .cpp
│   ├── pid.h / .cpp
│   ├── movement.h / .cpp
│   ├── maze.h / .cpp
│   ├── floodfill.h / .cpp
│   └── navigation.h / .cpp
│
├── tests/
│   ├── test_motors/
│   ├── test_encoders/
│   ├── test_mpu6050/
│   ├── test_ultrasonic/
│   ├── test_battery/
│   └── test_button/
│
└── docs/
    ├── PRD.md / TRD.md / Workflow.md
    ├── Pins.md / Circuit.md / Progress.md
    └── stages/  (stage1–stage7)
```

Main loop:

```
setup()
  → initAll()
  → pinMode(BUTTON_PIN, INPUT_PULLUP)

loop()
  → handleButton()     // debounced start/stop
  → updateYawAngle()
  → state machine:
      IDLE → CALIBRATING → EXPLORING → GOAL → SPEED_RUN
      (button press = emergency stop at any time)
```

---

# ⚠️ Biggest Problems Beginners Face

### 1️⃣ Movement not accurate

Solution:

```
tune PID
```

---

### 2️⃣ Ultrasonic noise

Solution:

```
take average of 5 readings
```

---

### 3️⃣ MPU drift

Solution:

```
use it only during turns
```

---

### 4️⃣ A6/A7 are analog-only pins

Solution:

```
reassign right ultrasonic to digital pins
or use analog threshold method
```

---

# 🔥 Recommended Development Order

DO NOT skip this order.

```
 1  Motor test              → tests/test_motors/
 2  Encoder test            → tests/test_encoders/
 3  MPU6050 yaw test        → tests/test_mpu6050/
 4  Ultrasonic test         → tests/test_ultrasonic/
 5  Push button test        → tests/test_button/
 6  Move exact distance     → movement.cpp
 7  Turn 90 degree          → movement.cpp + imu.cpp
 8  Wall detection          → sensors.cpp
 9  Maze memory             → maze.cpp
10  Flood fill simulation   → floodfill.cpp
11  Real maze exploration   → navigation.cpp
12  Speed run               → navigation.cpp
```

---

# 🧪 Testing Strategy

Before real robot:

Test algorithm in **simulator**.

Good ones:

- Micromouse Simulator
- MMS simulator (Qt)

---

# 🧠 How Antigravity Should Be Used

You should ask AI things like:

```
generate encoder based distance control for arduino nano
```

```
generate floodfill algorithm for 8x8 micromouse
```

```
update maze walls using ultrasonic sensors
```

```
generate PID straight line control for differential drive robot
```

```
generate micromouse wall update logic with neighbor mirroring
```

---

# 🏆 Final Micromouse Behavior

Robot will:

```
start
↓
explore maze
↓
reach center
↓
calculate shortest path
↓
speed run
```
