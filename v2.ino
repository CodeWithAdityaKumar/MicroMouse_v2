// Micromouse v2 — Main Entry Point
// See docs/PRD.md for full specification

#include "src/config.h"
#include "src/motors.h"
#include "src/encoders.h"
#include "src/imu.h"
#include "src/sensors.h"
#include "src/pid.h"
#include "src/movement.h"
#include "src/maze.h"
#include "src/floodfill.h"
#include "src/navigation.h"

// ─── Robot State ───────────────────────────────
BotState botState = STATE_IDLE;
uint8_t posX = 0, posY = 0;
Direction heading = NORTH;

unsigned long prevLoopTime = 0;

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println(F("Micromouse v2 — Initializing..."));

  initMotors();
  initEncoders();
  initSensors();
  initPID();
  initMaze();

  // Push button setup
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (initIMU()) {
    Serial.println(F("MPU6050 OK"));
  } else {
    Serial.println(F("MPU6050 FAIL"));
  }

  botState = STATE_IDLE;
  Serial.println(F("Ready. Send 's' to start."));
}

void loop() {
  // TODO: Implement state machine
  // See docs/PRD.md §7 for state machine design

  unsigned long now = micros();
  if (now - prevLoopTime < LOOP_INTERVAL_US) return;
  prevLoopTime = now;

  // Button check (runs every loop)
  handleButton();

  updateYawAngle();

  switch (botState) {
    case STATE_IDLE:
      // Wait for start command
      break;
    case STATE_CALIBRATING:
      // TODO: Run MPU calibration
      break;
    case STATE_EXPLORING:
      // TODO: Read walls → update maze → flood fill → decide → move
      break;
    case STATE_TURNING:
      // TODO: Execute turn
      break;
    case STATE_MOVING:
      // TODO: Execute forward movement
      break;
    case STATE_SPEED_RUN:
      // TODO: Replay stored path
      break;
    case STATE_STOPPED:
      brake();
      break;
  }
}

// ─── Button Handling ─────────────────────────────
static bool lastButtonState = HIGH;
static unsigned long lastDebounceTime = 0;

void handleButton() {
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
    static bool currentState = HIGH;
    if (reading != currentState) {
      currentState = reading;
      if (currentState == LOW) {
        onButtonPress();
      }
    }
  }

  lastButtonState = reading;
}

void onButtonPress() {
  switch (botState) {
    case STATE_IDLE:
      botState = STATE_CALIBRATING;
      Serial.println(F("Button: START"));
      break;
    case STATE_EXPLORING:
    case STATE_MOVING:
    case STATE_TURNING:
    case STATE_SPEED_RUN:
      botState = STATE_STOPPED;
      brake();
      Serial.println(F("Button: STOP"));
      break;
    case STATE_STOPPED:
      botState = STATE_IDLE;
      Serial.println(F("Button: RESET"));
      break;
    default:
      break;
  }
}
