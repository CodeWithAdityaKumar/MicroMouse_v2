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
