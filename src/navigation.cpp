#include "include/navigation.h"
#include "include/maze.h"
#include "include/sensors.h"
#include "include/motion.h"
#include "include/floodfill.h"
#include "include/imu.h"
#include <Arduino.h>

// ── Robot position state ─────────────────────────────────────────────────────
uint8_t   botX   = 0;
uint8_t   botY   = 0;
Direction botDir  = NORTH;

// Speed-run path (Stage 7)
#define MAX_PATH 128
static char   speedRunPath[MAX_PATH];
static uint8_t pathLength = 0;

// Neighbor offsets for N, E, S, W
static const int8_t dx[4] = {  0, +1,  0, -1 };
static const int8_t dy[4] = { +1,  0, -1,  0 };

// ─────────────────────────────────────────────────────────────────────────────
// initNavigation: reset bot state, maze, and compute first flood fill
// ─────────────────────────────────────────────────────────────────────────────
void initNavigation(uint8_t startX, uint8_t startY, Direction startHeading) {
  botX  = startX;
  botY  = startY;
  botDir = startHeading;

  initMaze();
  floodFill();
  pathLength = 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Direction Helpers
// ─────────────────────────────────────────────────────────────────────────────
Direction turnLeftDir(Direction d) { return (Direction)(((int)d + 3) % 4); }
Direction turnRightDir(Direction d) { return (Direction)(((int)d + 1) % 4); }
Direction turnBackDir(Direction d) { return (Direction)(((int)d + 2) % 4); }

// ─────────────────────────────────────────────────────────────────────────────
// turnToFace: Rotate the robot from `current` to `target` direction
// using the fewest turns (prefer right turn for equal cost)
// ─────────────────────────────────────────────────────────────────────────────
void turnToFace(Direction target, Direction current) {
  int diff = ((int)target - (int)current + 4) % 4;
  bool turned = false;
  
  switch (diff) {
    case 0: break;                // No turn needed
    case 1: turnRight90(); turned = true; break; // 90° clockwise
    case 2: turnBack();    turned = true; break; // 180°
    case 3: turnLeft90();  turned = true; break; // 90° counter-clockwise
  }
  
  // If we turned, we must let the robot physically settle before the next
  // moveForward() samples its baseYaw.
  if (turned) {
    delay(250); 
    updateYawAngle();
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// exploreStep: one full iteration of the explore loop
//   Returns true when the goal has been reached
// ─────────────────────────────────────────────────────────────────────────────
bool exploreStep() {
  // 1. Wait for robot to fully settle after previous turn/move before reading sensors
  delay(150);

  // 2. Take 3 samples per sensor and use majority vote to reject noise.
  int frontCount = 0, leftCount = 0, rightCount = 0;
  for (int i = 0; i < 3; i++) {
    readAllSensorsBlocking(); // Use guaranteed read when stopped
    if (distFront < MAZE_WALL_CM) frontCount++;
    if (distLeft  < MAZE_WALL_CM) leftCount++;
    if (distRight < MAZE_WALL_CM) rightCount++;
  }
  bool wFront = (frontCount >= 2);
  bool wLeft  = (leftCount  >= 2);
  bool wRight = (rightCount >= 2);

  // Log to Serial so user can verify sensor readings during debugging
  Serial.print(F("  Sensors: F="));
  Serial.print(distFront, 1);
  Serial.print(F(" L="));
  Serial.print(distLeft, 1);
  Serial.print(F(" R="));
  Serial.print(distRight, 1);
  Serial.print(F("  Walls: F="));
  Serial.print(wFront ? '1' : '0');
  Serial.print(F(" L="));
  Serial.print(wLeft  ? '1' : '0');
  Serial.print(F(" R="));
  Serial.println(wRight ? '1' : '0');

  // 3. Record walls detected from current cell/heading
  updateWalls(botX, botY, botDir, wFront, wLeft, wRight);

  // 4. Mark cell as visited
  maze[botX][botY].visited = true;

  // 5. Check if we have reached the goal
  if (isGoal(botX, botY)) {
    return true; // We reached the center and mapping is sufficiently done to stop here
  }

  // 6. Recalculate flood fill with new wall knowledge
  floodFill();

  // 7. Choose the best direction based on the updated distance map
  Direction nextDir = getBestDirection(botX, botY, botDir);

  // 8. Turn to face the target direction
  turnToFace(nextDir, botDir);
  botDir = nextDir;

  // 9. Move forward exactly one cell
  // We need to translate absolute maze walls (N/E/S/W) into relative (Left/Right)
  // based on the new heading the bot just turned to face.
  bool hasL = false, hasR = false;
  switch (botDir) {
    case NORTH: hasL = maze[botX][botY].walls & WALL_W; hasR = maze[botX][botY].walls & WALL_E; break;
    case EAST:  hasL = maze[botX][botY].walls & WALL_N; hasR = maze[botX][botY].walls & WALL_S; break;
    case SOUTH: hasL = maze[botX][botY].walls & WALL_E; hasR = maze[botX][botY].walls & WALL_W; break;
    case WEST:  hasL = maze[botX][botY].walls & WALL_S; hasR = maze[botX][botY].walls & WALL_N; break;
  }
  
  moveForward(CELL_SIZE_MM / 10.0f, hasL, hasR);

  // 10. Update position
  botX = (uint8_t)(botX + dx[botDir]);
  botY = (uint8_t)(botY + dy[botDir]);

  return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// computeSpeedRunPath: Trace optimal path from (0,0) to goal and 
// store as a sequence of F (Forward), L (Turn Left + Forward), R (Turn Right + Forward)
// ─────────────────────────────────────────────────────────────────────────────
void computeSpeedRunPath() {
  uint8_t currX = 0, currY = 0;
  Direction currHeading = NORTH; 
  pathLength = 0;
  
  // Make sure we start from a fresh floodfill map from the goal
  floodFill();

  while (!isGoal(currX, currY) && pathLength < MAX_PATH - 1) {
    Direction nextDir = getBestDirection(currX, currY, currHeading);
    
    // Determine the turn required
    int diff = ((int)nextDir - (int)currHeading + 4) % 4;
    
    // Note: We don't record the turn separately. 
    // F means "stay straight and go forward 1 cell"
    // R means "turn right, then go forward 1 cell"
    // L means "turn left, then go forward 1 cell"
    // (A 180 turn 'B' should not happen on an optimal path unless it's the very first move)
    if (diff == 0)      speedRunPath[pathLength++] = 'F';
    else if (diff == 1) speedRunPath[pathLength++] = 'R';
    else if (diff == 3) speedRunPath[pathLength++] = 'L';
    else if (diff == 2) speedRunPath[pathLength++] = 'B'; 
    
    currHeading = nextDir;
    currX += dx[currHeading];
    currY += dy[currHeading];
  }
  
  speedRunPath[pathLength] = '\0'; // Null terminate
  
  Serial.print(F("Optimal Path Length: "));
  Serial.println(pathLength);
  Serial.print(F("Path: "));
  Serial.println(speedRunPath);
}

// ─────────────────────────────────────────────────────────────────────────────
// executeSpeedRun: Replay stored path at high speed with straight-line compression
// ─────────────────────────────────────────────────────────────────────────────
void executeSpeedRun() {
  Serial.println(F("Executing Speed Run..."));
  
  // Start from completely known position
  botX = 0; botY = 0; botDir = NORTH;
  
  // Temporarily boost speed config if we had a dynamic variable
  // For now, we reuse moveForward which uses BASE_SPEED.
  // In a true micromouse, we would pass a higher max_pwm bound here.
  
  for (uint8_t i = 0; i < pathLength; ) {
    char cmd = speedRunPath[i];
    
    if (cmd == 'R') {
      turnRight90();
      botDir = turnRightDir(botDir);
      delay(200);
      updateYawAngle();
      i++;
    } 
    else if (cmd == 'L') {
      turnLeft90();
      botDir = turnLeftDir(botDir);
      delay(200);
      updateYawAngle();
      i++;
    }
    else if (cmd == 'B') {
      turnBack();
      botDir = turnBackDir(botDir);
      delay(200);
      updateYawAngle();
      i++;
    }
    
    // After any turn (or if we started with 'F'), we must go forward.
    // Count consecutive 'F's to compress them into a single continuous drive!
    int consecutiveForwards = 1; // We always go at least 1 cell for any command (R means turn, THEN go)
    if (cmd == 'F') {
      // It's just a forward. Let's look ahead for more 'F's.
      while (i + 1 < pathLength && speedRunPath[i + 1] == 'F') {
        consecutiveForwards++;
        i++;
      }
      i++; // consume the final 'F'
    } else {
      // It was a turn. It consumes 1 cell forward inherently in our encoding.
    }
    
    // We need to translate absolute maze walls (N/E/S/W) into relative (Left/Right)
    // for wall-centering safety during this straight.
    bool hasL = false, hasR = false;
    switch (botDir) {
      case NORTH: hasL = maze[botX][botY].walls & WALL_W; hasR = maze[botX][botY].walls & WALL_E; break;
      case EAST:  hasL = maze[botX][botY].walls & WALL_N; hasR = maze[botX][botY].walls & WALL_S; break;
      case SOUTH: hasL = maze[botX][botY].walls & WALL_E; hasR = maze[botX][botY].walls & WALL_W; break;
      case WEST:  hasL = maze[botX][botY].walls & WALL_S; hasR = maze[botX][botY].walls & WALL_N; break;
    }
    
    // Execute the compressed continuous movement
    float distanceCm = (CELL_SIZE_MM / 10.0f) * consecutiveForwards;
    moveForward(distanceCm, hasL, hasR);
    
    // Update structural bot position after multi-cell jump
    botX += dx[botDir] * consecutiveForwards;
    botY += dy[botDir] * consecutiveForwards;
    
    // Settle briefly after a massive straight before executing the next turn
    delay(100); 
  }
  
  Serial.println(F("\n*** SPEED RUN COMPLETE! ***"));
}
