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
