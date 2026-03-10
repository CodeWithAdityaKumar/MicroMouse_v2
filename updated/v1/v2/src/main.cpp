// ================================================================
//  MICROMOUSE MAZE-SOLVING FIRMWARE v2.1
//
//  Platform:      Arduino Nano (ATmega328P @ 16 MHz)
//  Motor Driver:  TB6612FNG
//  IMU:           MPU6050 (I²C)
//  Sensors:       3× Sharp GP2Y0A710K0F IR (left, front, right)
//  Encoders:      2× Quadrature (interrupt-driven)
//
//  Pin wiring matches mpu.ino and rotate90deg.ino exactly.
//  Motor functions are identical to those tested & proven files.
//
//  Flow:
//    Power on → auto gyro calibration → IDLE
//    Button press → heading lock + drive
//    Wall detected → decide: straight / left / right / U-turn
//    Button press → stop
// ================================================================

#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>

#include "config.h"
#include "WallReadings.h"


// ======================== FORWARD DECLARATIONS ========================
void onButtonPress();
void encLeftISR();
void encRightISR();


// ======================== IMU OBJECT ========================

MPU6050 mpu;


// ======================== STATE MACHINE ========================

enum BotState : uint8_t {
  STATE_IDLE,
  STATE_CALIBRATING,
  STATE_DRIVING,
  STATE_TURNING,
  STATE_DECIDING,
  STATE_STOPPED
};

volatile BotState botState = STATE_IDLE;


// ======================== GLOBAL VARIABLES ========================

// ---- Gyro / Heading ----
float gyroOffset        = 0.0;
float currentAngle      = 0.0;       // Integrated yaw (degrees)
float targetAngle       = 0.0;       // Desired heading (degrees)

// ---- PID State (Heading) ----
float pidPrevError      = 0.0;
float pidIntegral       = 0.0;
float pidOutput         = 0.0;

// ---- PD State (Wall Centering) ----
float prevWallError     = 0.0;

// ---- Timing ----
unsigned long prevTimeMicros  = 0;
float dt                      = 0.0;   // seconds

// ---- Encoders (ISR-shared — volatile) ----
volatile long encLeftTicks  = 0;
volatile long encRightTicks = 0;

// ---- Cell Tracking ----
long cellTicksSnapshot  = 0;          // Encoder avg at cell start

// ---- Turn State ----
unsigned long turnStartMs     = 0;
unsigned long turnSettleStart = 0;

// ---- Button Debounce ----
bool          lastRawButton     = HIGH;
bool          debouncedButton   = HIGH;
unsigned long lastDebounceTime  = 0;

// ---- Battery ----
unsigned long lastBattMs  = 0;
float battVoltage         = 12.0;     // Init to safe value
bool  lowBattery          = false;

// ---- Wall Readings ----
WallReadings walls;

// ---- Debug ----
int printCounter = 0;


// ================================================================
//  INTERRUPT SERVICE ROUTINES — Encoders
//  (Identical to mpu.ino / rotate90deg.ino)
// ================================================================

void encLeftISR()
{
  if (digitalRead(ENC_LEFT_B) == HIGH) {
    encLeftTicks++;
  } else {
    encLeftTicks--;
  }
}

void encRightISR()
{
  if (digitalRead(ENC_RIGHT_B) == HIGH) {
    encRightTicks++;
  } else {
    encRightTicks--;
  }
}


// ================================================================
//  MOTOR FUNCTIONS
//  (Identical to mpu.ino — same pin mapping, same logic)
// ================================================================

void setMotors(int left, int right)
{
  left  = constrain(left,  -MAX_SPEED, MAX_SPEED);
  right = constrain(right, -MAX_SPEED, MAX_SPEED);

  // Left motor (PWMA / AIN1 / AIN2 → D6 / D10 / D9)
  if (left >= 0) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, left);
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    analogWrite(PWMA, -left);
  }

  // Right motor (PWMB / BIN1 / BIN2 → D5 / D12 / D11)
  if (right >= 0) {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, right);
  } else {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    analogWrite(PWMB, -right);
  }
}

void stopMotors()
{
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, 0);

  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, 0);
}


// ================================================================
//  WALL SENSORS
// ================================================================

static int readSensorAvg(uint8_t pin)
{
  long sum = 0;
  for (uint8_t i = 0; i < SENSOR_SAMPLES; i++) {
    sum += analogRead(pin);
  }
  return (int)(sum / SENSOR_SAMPLES);
}

WallReadings readWallSensors()
{
  WallReadings w;

  w.leftRaw   = readSensorAvg(SENSOR_LEFT_PIN);
  w.frontRaw  = readSensorAvg(SENSOR_FRONT_PIN);
  w.rightRaw  = readSensorAvg(SENSOR_RIGHT_PIN);

  w.leftWall   = (w.leftRaw  > WALL_DETECT_THRESHOLD);
  w.frontWall  = (w.frontRaw > WALL_DETECT_THRESHOLD);
  w.rightWall  = (w.rightRaw > WALL_DETECT_THRESHOLD);

  w.leftClose  = (w.leftRaw  > WALL_CLOSE_THRESHOLD);
  w.frontClose = (w.frontRaw > WALL_CLOSE_THRESHOLD);
  w.rightClose = (w.rightRaw > WALL_CLOSE_THRESHOLD);

  w.leftDanger  = (w.leftRaw  > WALL_TOO_CLOSE);
  w.frontDanger = (w.frontRaw > WALL_TOO_CLOSE);
  w.rightDanger = (w.rightRaw > WALL_TOO_CLOSE);

  return w;
}


// ================================================================
//  GYRO CALIBRATION
//  (Matches mpu.ino — same sample count, same variance check)
// ================================================================

bool calibrateGyro()
{
  Serial.println(F("Calibrating gyro — keep bot still..."));

  long   sum   = 0;
  float  sumSq = 0;

  for (int i = 0; i < GYRO_CALIB_SAMPLES; i++) {
    int16_t gx, gy, gz;
    mpu.getRotation(&gx, &gy, &gz);
    sum   += gz;
    sumSq += (float)gz * (float)gz;
    delayMicroseconds(GYRO_CALIB_DELAY_US);
  }

  float mean     = sum / (float)GYRO_CALIB_SAMPLES;
  float variance = (sumSq / (float)GYRO_CALIB_SAMPLES) - (mean * mean);

  if (variance > GYRO_MAX_VARIANCE) {
    Serial.print(F("FAIL: Variance="));
    Serial.println(variance, 1);
    return false;
  }

  gyroOffset = mean;

  Serial.print(F("OK. Offset="));
  Serial.print(gyroOffset, 2);
  Serial.print(F("  Var="));
  Serial.println(variance, 1);
  return true;
}


// ================================================================
//  ANGLE UPDATE
//  (Matches mpu.ino — same integration method)
// ================================================================

void updateAngle()
{
  unsigned long now = micros();
  dt = (now - prevTimeMicros) / 1000000.0;
  prevTimeMicros = now;

  int16_t gx, gy, gz;
  mpu.getRotation(&gx, &gy, &gz);

  float gyroZ = ((float)gz - gyroOffset) / GYRO_SENSITIVITY;   // °/s
  currentAngle += gyroZ * dt;
}


// ================================================================
//  PID — HEADING CONTROLLER
//  (Matches mpu.ino PID structure)
// ================================================================

float computeHeadingPID()
{
  float error = targetAngle - currentAngle;

  // Integral with anti-windup
  pidIntegral += error * dt;
  pidIntegral = constrain(pidIntegral, -MAX_INTEGRAL_HEADING, MAX_INTEGRAL_HEADING);

  // Derivative (scaled by dt for frequency independence)
  float derivative = 0.0;
  if (dt > 0) {
    derivative = (error - pidPrevError) / dt;
  }

  pidOutput = (KP_HEADING * error) + (KI_HEADING * pidIntegral) + (KD_HEADING * derivative);

  // Clamp output
  pidOutput = constrain(pidOutput, -(float)MAX_SPEED, (float)MAX_SPEED);

  pidPrevError = error;

  return pidOutput;
}


// ================================================================
//  PD — WALL CENTERING
// ================================================================

float computeWallPD(const WallReadings &w)
{
  float wallError = 0.0;

  if (w.leftWall && w.rightWall) {
    wallError = (float)(w.leftRaw - w.rightRaw);
  }
  else if (w.leftWall) {
    wallError = (float)(w.leftRaw - WALL_CLOSE_THRESHOLD) * 0.5;
  }
  else if (w.rightWall) {
    wallError = (float)(WALL_CLOSE_THRESHOLD - w.rightRaw) * 0.5;
  }

  float dError = wallError - prevWallError;
  prevWallError = wallError;

  return (KP_WALL * wallError) + (KD_WALL * dError);
}


// ================================================================
//  GYRO DRIFT CORRECTION — Wall Alignment
// ================================================================

void wallAlignDriftCorrection(const WallReadings &w)
{
  if (w.leftWall && w.rightWall) {
    float imbalance = (float)(w.leftRaw - w.rightRaw);
    currentAngle += WALL_ALIGN_GAIN * imbalance * 0.01;
  }
}


// ================================================================
//  ENCODER HELPERS
// ================================================================

static void getEncoderTicks(long &left, long &right)
{
  noInterrupts();
  left  = encLeftTicks;
  right = encRightTicks;
  interrupts();
}

void resetCellTracking()
{
  long l, r;
  getEncoderTicks(l, r);
  cellTicksSnapshot = (l + r) / 2;
}

// Returns ticks traveled since last cell reset
long getCellTicks()
{
  long l, r;
  getEncoderTicks(l, r);
  return (l + r) / 2 - cellTicksSnapshot;
}


// ================================================================
//  TURN EXECUTION (Non-blocking — driven by heading PID)
// ================================================================

static float snapToCardinal(float angle)
{
  float snapped = round(angle / 90.0) * 90.0;
  if (snapped >  180.0) snapped -= 360.0;
  if (snapped < -180.0) snapped += 360.0;
  return snapped;
}

void startTurn(float deltaDeg, const __FlashStringHelper* label)
{
  stopMotors();

  targetAngle += deltaDeg;
  // Snap to nearest cardinal to prevent drift accumulation
  targetAngle = snapToCardinal(targetAngle);

  // Reset PID state for clean turn
  turnStartMs     = millis();
  turnSettleStart = 0;
  pidIntegral     = 0.0;
  pidPrevError    = 0.0;

  botState = STATE_TURNING;

  Serial.print(F(">> "));
  Serial.print(label);
  Serial.print(F(" target="));
  Serial.println(targetAngle, 1);
}

void turnLeft90()   { startTurn(+90.0,  F("TURN LEFT 90"));  }
void turnRight90()  { startTurn(-90.0,  F("TURN RIGHT 90")); }
void turnAround()   { startTurn(+180.0, F("U-TURN 180"));    }

bool checkTurnSettled(float headingError)
{
  // Safety timeout
  if ((millis() - turnStartMs) > TURN_TIMEOUT_MS) {
    Serial.println(F(">> Turn TIMEOUT"));
    return true;
  }

  if (abs(headingError) < TURN_SETTLE_RATE) {
    if (turnSettleStart == 0) {
      turnSettleStart = millis();
    } else if ((millis() - turnSettleStart) >= TURN_SETTLE_TIME_MS) {
      return true;
    }
  } else {
    turnSettleStart = 0;
  }
  return false;
}


// ================================================================
//  NAVIGATION — Left-Hand Wall-Following
// ================================================================

void makeNavigationDecision()
{
  walls = readWallSensors();

  Serial.print(F("WALLS: L="));
  Serial.print(walls.leftWall  ? F("YES") : F("no "));
  Serial.print(F(" F="));
  Serial.print(walls.frontWall ? F("YES") : F("no "));
  Serial.print(F(" R="));
  Serial.println(walls.rightWall ? F("YES") : F("no "));

  if (!walls.frontWall) {
    Serial.println(F(">> GO STRAIGHT"));
    resetCellTracking();
    botState = STATE_DRIVING;
  }
  else if (!walls.leftWall) {
    turnLeft90();
  }
  else if (!walls.rightWall) {
    turnRight90();
  }
  else {
    turnAround();
  }
}


// ================================================================
//  BUTTON HANDLER (Debounced)
//  (Same debounce logic as mpu.ino / rotate90deg.ino)
// ================================================================

void handleButton()
{
  bool rawReading = digitalRead(BUTTON_PIN);

  if (rawReading != lastRawButton) {
    lastDebounceTime = millis();
  }
  lastRawButton = rawReading;

  if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
    if (debouncedButton == HIGH && rawReading == LOW) {
      Serial.println(F(""));
      Serial.println(F("*** BUTTON PRESSED ***"));
      onButtonPress();
    }
    debouncedButton = rawReading;
  }
}

void onButtonPress()
{
  switch (botState)
  {
    // ---- IDLE or STOPPED → lock heading and start driving ----
    //   (Same as mpu.ino: single button press starts driving)
    case STATE_IDLE:
    case STATE_STOPPED:
    {
      targetAngle  = currentAngle;
      pidIntegral  = 0.0;
      pidPrevError = 0.0;
      prevWallError = 0.0;

      noInterrupts();
      encLeftTicks  = 0;
      encRightTicks = 0;
      interrupts();
      resetCellTracking();

      botState = STATE_DRIVING;
      Serial.println(F(">> RUNNING — heading locked"));
      Serial.print(F("   Target: "));
      Serial.println(targetAngle, 2);
      break;
    }

    // ---- Running → stop ----
    case STATE_DRIVING:
    case STATE_TURNING:
    case STATE_DECIDING:
    {
      stopMotors();
      botState = STATE_STOPPED;
      Serial.println(F(">> STOPPED by button"));
      break;
    }

    default:
      break;
  }
}


// ================================================================
//  BATTERY MONITOR
// ================================================================

void checkBattery()
{
  if ((millis() - lastBattMs) < BATT_READ_INTERVAL_MS) return;
  lastBattMs = millis();

  int raw = analogRead(BATT_PIN);
  battVoltage = (float)raw * (5.0 / 1023.0) * VBAT_SCALE;

  if (battVoltage < VBAT_MIN && !lowBattery) {
    lowBattery = true;
    stopMotors();
    botState = STATE_STOPPED;
    Serial.print(F("!! LOW BATTERY: "));
    Serial.print(battVoltage, 2);
    Serial.println(F("V — SAFE STOP"));
  }
}


// ================================================================
//  DEBUG OUTPUT
// ================================================================

void printDebug(float headingCorr, float wallCorr)
{
  printCounter++;
  if (printCounter < DEBUG_PRINT_INTERVAL) return;
  printCounter = 0;

  long l, r;
  getEncoderTicks(l, r);

  Serial.print(F("S:"));
  switch (botState) {
    case STATE_IDLE:    Serial.print(F("IDL")); break;
    case STATE_DRIVING: Serial.print(F("DRV")); break;
    case STATE_TURNING: Serial.print(F("TRN")); break;
    case STATE_DECIDING:Serial.print(F("DEC")); break;
    case STATE_STOPPED: Serial.print(F("STP")); break;
    default:            Serial.print(F("???")); break;
  }

  Serial.print(F(" A:"));
  Serial.print(currentAngle, 1);
  Serial.print(F(" T:"));
  Serial.print(targetAngle, 1);
  Serial.print(F(" HC:"));
  Serial.print(headingCorr, 1);
  Serial.print(F(" WC:"));
  Serial.print(wallCorr, 1);

  Serial.print(F(" EL:"));
  Serial.print(l);
  Serial.print(F(" ER:"));
  Serial.print(r);

  Serial.print(F(" WL:"));
  Serial.print(walls.leftRaw);
  Serial.print(F(" WF:"));
  Serial.print(walls.frontRaw);
  Serial.print(F(" WR:"));
  Serial.println(walls.rightRaw);
}


// ================================================================
//  SETUP
//  (Follows mpu.ino pattern: STBY=HIGH immediately, auto-calibrate)
// ================================================================

void setup()
{
  Serial.begin(SERIAL_BAUD);
  Serial.println(F("\n=== MICROMOUSE v2.1 ==="));

  // ---- I²C + MPU6050 ----
  Wire.begin();
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println(F("FATAL: MPU6050 not found!"));
    while (1) { delay(1000); }
  }
  Serial.println(F("MPU6050 OK."));

  // ---- Motor pins ----
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);    // *** ENABLE MOTORS IMMEDIATELY ***
  stopMotors();                 // Braked but driver is active

  // ---- Encoder pins ----
  pinMode(ENC_LEFT_A,  INPUT_PULLUP);
  pinMode(ENC_LEFT_B,  INPUT_PULLUP);
  pinMode(ENC_RIGHT_A, INPUT_PULLUP);
  pinMode(ENC_RIGHT_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_LEFT_A),  encLeftISR,  RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_RIGHT_A), encRightISR, RISING);

  // ---- Button ----
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // ---- Sensor check ----
  Serial.println(F("\n--- SENSOR CHECK (3 sec) ---"));
  unsigned long calStart = millis();
  while ((millis() - calStart) < SENSOR_CAL_DURATION_MS) {
    WallReadings w = readWallSensors();
    Serial.print(F("L="));
    Serial.print(w.leftRaw);
    Serial.print(F("\tF="));
    Serial.print(w.frontRaw);
    Serial.print(F("\tR="));
    Serial.print(w.rightRaw);
    Serial.print(F("\t| L"));
    Serial.print(w.leftWall  ? F("+") : F("-"));
    Serial.print(F(" F"));
    Serial.print(w.frontWall ? F("+") : F("-"));
    Serial.print(F(" R"));
    Serial.println(w.rightWall ? F("+") : F("-"));
    delay(200);
  }
  Serial.println(F("--- SENSOR CHECK DONE ---\n"));

  // ---- Gyro calibration (auto, up to 3 attempts — same as mpu.ino) ----
  botState = STATE_CALIBRATING;
  bool calibrated = false;
  for (int attempt = 1; attempt <= GYRO_CALIB_ATTEMPTS; attempt++) {
    Serial.print(F("Cal attempt "));
    Serial.print(attempt);
    Serial.println(F("/3"));
    if (calibrateGyro()) {
      calibrated = true;
      break;
    }
    delay(500);
  }

  if (!calibrated) {
    Serial.println(F("FATAL: Calibration failed. Halting."));
    botState = STATE_STOPPED;
    while (1) { delay(1000); }
  }

  // ---- Ready ----
  prevTimeMicros = micros();
  currentAngle = 0;
  botState = STATE_IDLE;

  Serial.println(F("Ready. Press button to start driving."));
}


// ================================================================
//  MAIN LOOP — 200 Hz non-blocking
//  (Same timing gate as mpu.ino)
// ================================================================

void loop()
{
  // ---- Button always active ----
  handleButton();

  // ---- 200 Hz timing gate ----
  unsigned long now = micros();
  if ((now - prevTimeMicros) < LOOP_INTERVAL_US) return;

  // ---- Update heading from gyro ----
  updateAngle();

  // ---- Battery monitor (low priority, ~1 Hz) ----
  checkBattery();
  if (lowBattery) return;

  // ---- State machine ----
  switch (botState)
  {
    // ======== DRIVING FORWARD ========
    case STATE_DRIVING:
    {
      // Read walls
      walls = readWallSensors();

      // Wall-alignment drift correction
      wallAlignDriftCorrection(walls);

      // Check front wall → stop and decide
      if (walls.frontRaw > WALL_FRONT_STOP) {
        stopMotors();
        botState = STATE_DECIDING;
        Serial.println(F(">> FRONT WALL — deciding..."));
        break;
      }

      // Check cell complete
      if (getCellTicks() >= ONE_CELL_TICKS) {
        stopMotors();
        botState = STATE_DECIDING;
        Serial.println(F(">> CELL COMPLETE — deciding..."));
        break;
      }

      // Heading PID
      float headingCorr = computeHeadingPID();

      // Wall centering PD
      float wallCorr = computeWallPD(walls);

      // Combined correction → motors
      float totalCorr = headingCorr + wallCorr;
      int leftSpeed  = BASE_SPEED + (int)totalCorr;
      int rightSpeed = BASE_SPEED - (int)totalCorr;
      leftSpeed  = constrain(leftSpeed,  MIN_SPEED, MAX_SPEED);
      rightSpeed = constrain(rightSpeed, MIN_SPEED, MAX_SPEED);
      setMotors(leftSpeed, rightSpeed);

      // Debug
      printDebug(headingCorr, wallCorr);
      break;
    }

    // ======== TURNING (Non-blocking pivot) ========
    case STATE_TURNING:
    {
      float turnError = targetAngle - currentAngle;

      if (checkTurnSettled(turnError)) {
        stopMotors();
        Serial.print(F(">> Turn COMPLETE angle="));
        Serial.println(currentAngle, 1);
        resetCellTracking();
        botState = STATE_DRIVING;
        break;
      }

      // PID drives the pivot
      float headingCorr = computeHeadingPID();
      int turnPower = constrain((int)headingCorr, -TURN_SPEED, TURN_SPEED);
      setMotors(turnPower, -turnPower);

      printDebug(headingCorr, 0);
      break;
    }

    // ======== DECIDING ========
    case STATE_DECIDING:
    {
      makeNavigationDecision();
      break;
    }

    // ======== IDLE / STOPPED ========
    case STATE_IDLE:
    case STATE_STOPPED:
    case STATE_CALIBRATING:
    default:
      // Motors already stopped; keep updating angle for readiness
      break;
  }
}
