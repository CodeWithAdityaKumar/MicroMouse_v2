# Micromouse v1 — Todo

> **Last Updated:** 2026-03-02

---

## Phase 1: Hardware Verification

- [x] Motor test — both motors spin forward, reverse, stop
- [x] IMU test — MPU6050 gyro X-axis = yaw (horizontal mount), calibration + PID working
- [ ] IR sensor test — deferred (noisy readings, needs hardware debug)
- [/] Encoder test — tick counting on both wheels (forward + reverse)
- [ ] Button test — debounced short press & long press detection
- [ ] Power test — verify stable 5V from regulator under motor load

---

## Phase 2: Drive Straight with PID

- [ ] Implement `config.h` with all pin definitions and constants
- [ ] Write `imu.h/.cpp` — init, calibrate, yaw integration
- [ ] Write `motor.h/.cpp` — `setMotors()`, `stopMotors()`, `enableMotors()`
- [ ] Write `pid.h/.cpp` — generic PID class with windup guard
- [ ] Implement gyro calibration (500-sample bias at startup)
- [ ] Implement heading PID loop (200 Hz, non-blocking)
- [ ] Calibrate PID gains (Kp, Ki, Kd) — robot drives straight 5 cells
- [ ] Verify: heading deviation < ±2° over 90 cm

---

## Phase 3: Turn Execution

- [ ] Implement 90° pivot turn (gyro-guided, settle detection)
- [ ] Implement 180° pivot turn
- [ ] Calibrate turn speed and settle thresholds
- [ ] Verify: 20 consecutive 90° turns with < ±3° error
- [ ] Verify: settle time < 500 ms per turn

---

## Phase 4: Wall Sensing & Classification

- [ ] Write `sensors.h/.cpp` — analog read, ADC→cm, averaging
- [ ] Create `WallReadings.h` struct (separate header to avoid prototype bugs)
- [ ] Calibrate IR sensors at known distances (5, 10, 15, 20, 30 cm)
- [ ] Implement wall threshold with hysteresis (12/15 cm band)
- [ ] Implement wall centering PID (IR left–right delta)
- [ ] Verify: ≥ 95% wall classification accuracy in test cell

---

## Phase 5: Encoder Odometry

- [ ] Write `encoder.h/.cpp` — ISR handlers, tick counters, reset
- [ ] Calibrate `TICKS_PER_CELL` (drive 18 cm, count ticks × 5 trials)
- [ ] Implement cell-boundary detection (average of left + right ticks)
- [ ] Verify: stops at cell boundary ±5 mm over 10 trials

---

## Phase 6: Maze Mapping & Flood Fill

- [ ] Write `maze.h/.cpp` — 16×16 cell array, wall storage
- [ ] Implement wall update logic (set walls for current cell + neighbour)
- [ ] Write `navigation.h/.cpp` — flood fill BFS
- [ ] Implement BFS queue (circular buffer, 256 entries)
- [ ] Implement `getNextDirection()` — pick lowest flood value neighbour
- [ ] Verify flood fill on paper / Serial debug output
- [ ] Test on small 4×4 physical maze

---

## Phase 7: State Machine & Integration

- [ ] Write `state_machine.h/.cpp` — all state transitions
- [ ] Write `button.h/.cpp` — debounced short/long press
- [ ] Implement full state flow: IDLE → CALIBRATING → HEADING_LOCK → EXPLORING
- [ ] Implement exploration loop (drive cell → read walls → flood → decide → turn → repeat)
- [ ] Implement goal detection (reached centre 4 cells)
- [ ] Implement return-to-start (reverse flood fill, goal = 0,0)
- [ ] Implement STOPPED state (long-press abort from any state)
- [ ] Integration test on 8×8 maze
- [ ] Full test on 16×16 maze

---

## Phase 8: Speed Run (Post-MVP)

- [ ] Store optimal path after exploration
- [ ] Implement speed run mode with higher base PWM
- [ ] Add button-triggered speed run from start position
- [ ] Tune PID for higher speeds
- [ ] Verify: speed run ≥ 30% faster than exploration

---

## Phase 9: Polish & Competition Prep (Post-MVP)

- [ ] Battery voltage monitoring on A3 (voltage divider)
- [ ] Graceful low-battery shutdown
- [ ] EEPROM map persistence (save/load maze)
- [ ] Serial debug toggle (compile-time flag)
- [ ] Final PID tuning pass
- [ ] Reliability stress test (10 consecutive solves)
- [ ] Measure and log solve times
- [ ] Mechanical adjustments (sensor angles, wheel grip, weight balance)

---

## Bugs & Issues

_None yet — add issues here as they come up._

---

## Notes

- Always test on a flat, smooth surface
- Recalibrate gyro if robot was jostled before start
- IR sensors need line-of-sight to maze walls (no obstructions)
- Keep battery charged above 7.0V for consistent motor performance
