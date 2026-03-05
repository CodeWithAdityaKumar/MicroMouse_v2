# 📊 Micromouse v2 — Development Progress

> **Last Updated**: 2026-03-05

---

## Stage 1 — Hardware Setup

### Component Tests

| #   | Test                   |   Status   | Date       |
| --- | ---------------------- | :--------: | ---------- |
| 1   | Motor forward/backward |  ✅ Done   | 2026-03-05 |
| 2   | Encoder tick count     | ⬜ Pending |            |
| 3   | MPU6050 yaw angle      |  ✅ Done   | 2026-03-05 |
| 4   | Ultrasonic distance    | ⬜ Pending |            |
| 5   | Push button            |  ✅ Done   | 2026-03-05 |

---

## Stage 2 — Motion Control

| #   | Test                | Status |
| --- | ------------------- | :----: |
| 1   | Move 1 cell (16 cm) |   ⬜   |
| 2   | Straight-line PID   |   ⬜   |
| 3   | Turn 90° left       |   ⬜   |
| 4   | Turn 90° right      |   ⬜   |
| 5   | Turn 180°           |   ⬜   |

---

## Stage 3 — Wall Detection

| #   | Test                       | Status |
| --- | -------------------------- | :----: |
| 1   | Front wall detect          |   ⬜   |
| 2   | Side wall detect           |   ⬜   |
| 3   | All sensors simultaneously |   ⬜   |

---

## Stage 4 — Maze Representation

| #   | Test                      | Status |
| --- | ------------------------- | :----: |
| 1   | Initialize maze grid      |   ⬜   |
| 2   | Update walls from sensors |   ⬜   |
| 3   | Print maze over Serial    |   ⬜   |

---

## Stage 5 — Flood Fill

| #   | Test                            | Status |
| --- | ------------------------------- | :----: |
| 1   | Flood fill on empty maze        |   ⬜   |
| 2   | Flood fill with walls           |   ⬜   |
| 3   | Decision picks correct neighbor |   ⬜   |

---

## Stage 6 — Exploration

| #   | Test                       | Status |
| --- | -------------------------- | :----: |
| 1   | Simple maze (no dead-ends) |   ⬜   |
| 2   | Complex maze (dead-ends)   |   ⬜   |
| 3   | Complete exploration       |   ⬜   |

---

## Stage 7 — Speed Run

| #   | Test           | Status |
| --- | -------------- | :----: |
| 1   | Path storage   |   ⬜   |
| 2   | Path replay    |   ⬜   |
| 3   | Speed increase |   ⬜   |
