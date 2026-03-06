# 📊 Micromouse v2 — Development Progress

> **Last Updated**: 2026-03-05

---

## Stage 1 — Hardware Setup

### Component Tests

| #   | Test                   | Status  | Date       |
| --- | ---------------------- | :-----: | ---------- |
| 1   | Motor forward/backward | ✅ Done | 2026-03-05 |
| 2   | Encoder tick count     | ✅ Done | 2026-03-05 |
| 3   | MPU6050 yaw angle      | ✅ Done | 2026-03-05 |
| 4   | Ultrasonic distance    | ✅ Done | 2026-03-05 |
| 5   | Push button            | ✅ Done | 2026-03-05 |

---

## Stage 2 — Motion Control

| #   | Test                | Status  |
| --- | ------------------- | :-----: |
| 1   | Move 1 cell (16 cm) | ✅ Done |
| 2   | Straight-line PID   | ✅ Done |
| 3   | Turn 90° left       | ✅ Done |
| 4   | Turn 90° right      | ✅ Done |
| 5   | Turn 180°           | ✅ Done |

---

## Stage 3 — Wall Detection

| #   | Test                       | Status  |
| --- | -------------------------- | :-----: |
| 1   | Front wall detect          | ✅ Done |
| 2   | Side wall detect           | ✅ Done |
| 3   | All sensors simultaneously | ✅ Done |

---

## Stage 4 — Maze Representation

| #   | Test                      | Status  |
| --- | ------------------------- | :-----: |
| 1   | Initialize maze grid      | ✅ Done |
| 2   | Update walls from sensors | ✅ Done |
| 3   | Print maze over Serial    | ✅ Done |

---

## Stage 5 — Flood Fill

| #   | Test                            | Status  |
| --- | ------------------------------- | :-----: |
| 1   | Flood fill on empty maze        | ✅ Done |
| 2   | Flood fill with walls           | ✅ Done |
| 3   | Decision picks correct neighbor | ✅ Done |

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
