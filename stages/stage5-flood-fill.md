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
