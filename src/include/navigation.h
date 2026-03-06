#pragma once

#include <stdint.h>
#include "config.h"
#include "maze.h"

// ── Robot position state (exported so test sketches can read them) ──────────
extern uint8_t  botX;
extern uint8_t  botY;
extern Direction botDir;

// ── Lifecycle ────────────────────────────────────────────────────────────────
void initNavigation();

// ── Exploration ──────────────────────────────────────────────────────────────
// Execute exactly one explore step:
//   read sensors → update walls → flood fill → choose direction → turn → move
// Returns true when the goal has been reached.
bool exploreStep();

// ── Turn helper ──────────────────────────────────────────────────────────────
// Turn the bot to face `target` from `current` using the shortest rotation.
void turnToFace(Direction target, Direction current);

// ── Speed-run (Stage 7) stubs ────────────────────────────────────────────────
void computeSpeedRunPath();
void executeSpeedRun();
