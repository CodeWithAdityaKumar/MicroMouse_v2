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
