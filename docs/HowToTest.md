# Micromouse Testing Guide

This document outlines the testing procedures for each stage, detailing how to run the test and the exact criteria for Pass/Fail.

---

## Stage 2: Motion Control

### moveForward (1 cell)

- **Test File:** `tests/stage2/test_motion/test_motion.ino`
- **Procedure:** Place the bot on the floor. Click the button to calibrate, click again to run.
- **Pass:** The robot moves exactly 16cm (1 cell) forward and stops completely. It does not veer heavily.
- **Fail:** The robot moves significantly more or less than 16cm, rotates wildly, or fails to stop.

### Straight-line PID

- **Test File:** `tests/stage2/test_straight_pid/test_straight_pid.ino`
- **Procedure:** Place the bot on a straight line or maze straightaway. Run the test.
- **Pass:** The robot drives forward 2+ cells while maintaining a perfectly straight line, wiggling slightly if it corrects itself.
- **Fail:** The robot gradually veers left or right and drifts into a wall. Solution: Tune `STRAIGHT_KP` in `config.h`.

### Turning (90 Left/Right, 180 Back)

- **Test File:** `tests/stage2/test_motion/test_motion.ino`
- **Procedure:** Run the test.
- **Pass:** The robot cleanly rotates in place, stopping precisely at 90°, -90°, and 180° utilizing the MPU6050 yaw angle.
- **Fail:** The robot overshoots or undershoots the turn significantly. Solution: Give the MPU6050 more settle time or lower the `MIN_TURN_SPEED`/Kp.

---

## Stage 3: Wall Detection

### Ultrasonic Sensor Polling

- **Test File:** `tests/stage3/test_sensors/test_sensors.ino`
- **Procedure:** Open Serial Monitor at 115200 baud. Place hand in front of Left, Front, and Right sensors one by one.
- **Pass:** The Serial Monitor shows accurately measured distances (in cm) for each sensor. When a hand is < 10cm, the monitor explicitly says `WALL`. No false positive echoes on other sensors.
- **Fail:** Monitor reports `OOR` (Out of Range) incorrectly, or the script freezes/stalls due to echo timeout.

### Dynamic Wall Centering PID

- **Test File:** `tests/stage3/test_wall_centering/test_wall_centering.ino`
- **Procedure:** Place the bot on the floor. Run the test, and while the bot is driving forward, place your hand 2cm away from its left or right side.
- **Pass:** The robot aggressively steers _away_ from your hand to maintain exactly 3.5cm distance, and then uses the IMU to recover a straight heading without crashing.
- **Fail:** The robot steers _into_ your hand, or successfully avoids your hand but then continues driving diagonally off course instead of recovering straight.

---

## Stage 4: Maze Representation

### Maze Grid / Wall Memory

- **Test File:** `tests/stage4/test_maze/test_maze.ino`
- **Procedure:** Upload the test sketch and open the Serial Monitor at 115200 baud. **No motors run** — this is a pure logic test.
- **Pass Criteria:**
  1. **Initial Maze (before simulation):** The outer border of the 8×8 grid must show walls on all 4 edges with `+---+` lines. All inner cells should show `?` (unvisited).
  2. **Updated Maze (after simulation):** The simulated wall readings at cells `(0,0)`, `(0,1)`, and `(1,1)` must appear correctly as `+---` segments in the ASCII drawing. Shared walls between adjacent cells must be mirrored (if a wall appears on the East side of `(0,0)`, the West side of `(1,0)` must also show a wall).
  3. **Distance Numbers:** Cells `(0,0)`, `(0,1)`, and `(1,1)` must show distances `0`, `1`, and `2` respectively.
  4. **Goal Check:** The Serial Monitor must print `PASS` for `isGoal(3,3)` and `isGoal(4,4)`, and `PASS (not goal)` for `isGoal(0,0)`.
- **Fail:** Any wall that was registered on one cell is NOT mirrored in the neighboring cell. Or the goal coordinates don't match the configured values in `config.h`.

---

## Stage 5: Flood Fill Algorithm

### Flood Fill BFS + Direction Selection

- **Test File:** `tests/stage5/test_floodfill/test_floodfill.ino`
- **Procedure:** Upload the test sketch and open the Serial Monitor at 115200 baud. **No motors run** — this is a pure logic test.
- **Pass Criteria:**

  **Test 1 — Empty Maze:**
  1. The ASCII grid prints with `0` inside all 4 goal cells `(3,3)`, `(3,4)`, `(4,3)`, `(4,4)`.
  2. All 4 corners `(0,0)`, `(7,0)`, `(0,7)`, `(7,7)` show distance `7`.
  3. Serial Monitor prints: `maze[3][3].distance == 0: PASS`, `maze[0][0].distance == 7: PASS`, etc.

  **Test 2 — Walled Maze:**
  1. After adding a vertical wall between columns 3 and 4 in the center rows, the flood fill re-runs.
  2. Cells to the west of the wall must show distances `1` higher than before (because they must detour around the wall).
  3. Serial Monitor prints: `maze[2][3].distance (should be 2): PASS`

  **Test 3 — getBestDirection:**
  1. From corner `(0,0)` facing NORTH, the function must return `NORTH` or `EAST` (both are equal distance from center on an empty maze).
  2. Serial Monitor prints: `=> PASS`

- **Fail:** Any PASS/FAIL check prints `FAIL`. Or distances at goal cells are not `0`, or corners are not `7`.
