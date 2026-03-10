// ================================================================
//  MPU HEADING CORRECTION — Production Firmware
//  Platform:      Arduino Nano (ATmega328P)
//  Motor Driver:  TB6612FNG
//  IMU:           MPU6050 (gyro X-axis for yaw — mounted horizontally)
//  Encoders:      2x Rotary encoders (interrupt-driven)
//
//  Purpose: Drive the bot straight. If the heading drifts
//           (detected by the MPU6050 gyroscope), a PID
//           controller corrects the left/right motor speeds
//           to realign the bot to its locked heading.
// ================================================================

#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// ======================== FUNCTION PROTOTYPES ========================
void onButtonPress();

// ======================== PIN DEFINITIONS ========================
// Motor Driver — TB6612FNG (per Pins.md)
#define PWMA  5     // Left motor speed (PWM)
#define AIN1  12    // Left motor dir A
#define AIN2  11    // Left motor dir B

#define PWMB  6     // Right motor speed (PWM)
#define BIN1  10    // Right motor dir A
#define BIN2  9     // Right motor dir B

#define STBY  13    // Standby (HIGH = enabled)

// Button
#define BUTTON_PIN  4   // INPUT_PULLUP — press to start/reset

// Encoders (using hardware interrupt pins)
#define ENC_LEFT_A   2  // INT0 — left encoder channel A
#define ENC_LEFT_B   7  // left encoder channel B (direction)
#define ENC_RIGHT_A  3  // INT1 — right encoder channel A
#define ENC_RIGHT_B  8  // right encoder channel B (direction)

// ======================== CONFIGURATION ========================

// Gyroscope
#define GYRO_SENSITIVITY    131.0   // LSB/°/s at ±250°/s range
#define CALIB_SAMPLES       2000    // gyro calibration sample count
#define CALIB_DELAY_US      2000    // 2 ms per sample → ~4 seconds total

// Control Loop
#define LOOP_INTERVAL_US    5000    // 5 ms → 200 Hz control loop
#define BASE_SPEED          0       // no forward motion — correction only (pivot in place)
#define MAX_SPEED           255     // absolute motor cap
#define MIN_SPEED           0       // minimum meaningful PWM

// PID Gains — Heading correction
#define KP  5.5
#define KI  0.0
#define KD  1.8

// PID output clamp
#define PID_OUTPUT_MAX  (MAX_SPEED - BASE_SPEED)
#define PID_OUTPUT_MIN  (-(MAX_SPEED - BASE_SPEED))

// Integral windup guard
#define INTEGRAL_MAX  500.0

// Encoder
#define TICKS_PER_REV  20

// Debug — print every 200th loop → ~1 Hz at 200 Hz loop (slow printing)
#define DEBUG_PRINT_INTERVAL  200

// ======================== STATE MACHINE ========================

enum BotState {
  STATE_IDLE,
  STATE_CALIBRATING,
  STATE_RUNNING,
  STATE_STOPPED
};

volatile BotState botState = STATE_IDLE;

// ======================== GLOBAL VARIABLES ========================

// Gyro / Heading
float currentAngle    = 0.0;
float targetAngle     = 0.0;
float gyroOffset      = 0.0;

unsigned long prevTimeMicros = 0;
float dt = 0.0;

// PID state
float pidError     = 0.0;
float pidPrevError = 0.0;
float pidIntegral  = 0.0;
float pidDerivative = 0.0;
float pidOutput    = 0.0;

// Encoders (volatile — modified in ISR)
volatile long encLeftTicks  = 0;
volatile long encRightTicks = 0;

// Debug
int printCounter = 0;

// Button debounce
bool lastRawButton     = HIGH;
bool debouncedButton   = HIGH;
unsigned long lastDebounceTime = 0;
#define DEBOUNCE_MS  50

// ======================== INTERRUPT SERVICE ROUTINES ========================

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

// ======================== MOTOR FUNCTIONS ========================

void setMotors(int left, int right)
{
  left  = constrain(left,  -MAX_SPEED, MAX_SPEED);
  right = constrain(right, -MAX_SPEED, MAX_SPEED);

  // Left motor (PWMB / BIN1 / BIN2)
  if (left >= 0) {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, left);
  } else {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    analogWrite(PWMB, -left);
  }

  // Right motor (PWMA / AIN1 / AIN2)
  if (right >= 0) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, right);
  } else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    analogWrite(PWMA, -right);
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

// ======================== GYRO CALIBRATION ========================

bool calibrateGyro()
{
  Serial.println(F("Calibrating gyro — keep bot still..."));

  long sum = 0;
  float sumSq = 0;

  for (int i = 0; i < CALIB_SAMPLES; i++) {
    int16_t gx, gy, gz;
    mpu.getRotation(&gx, &gy, &gz);
    sum += gx;  // X-axis is yaw (MPU mounted horizontally)
    sumSq += (float)gx * (float)gx;
    delayMicroseconds(CALIB_DELAY_US);
  }

  float mean = sum / (float)CALIB_SAMPLES;
  float variance = (sumSq / (float)CALIB_SAMPLES) - (mean * mean);

  if (variance > 1000.0) {
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

// ======================== ANGLE UPDATE ========================

void updateAngle()
{
  unsigned long now = micros();
  dt = (now - prevTimeMicros) / 1000000.0;
  prevTimeMicros = now;

  int16_t gx, gy, gz;
  mpu.getRotation(&gx, &gy, &gz);

  // X-axis is yaw because MPU is mounted horizontally
  float gyroRate = (gx - gyroOffset) / GYRO_SENSITIVITY;  // °/s

  currentAngle += gyroRate * dt;
}

// ======================== PID CONTROLLER ========================

float computePID()
{
  pidError = targetAngle - currentAngle;

  pidIntegral += pidError * dt;
  pidIntegral = constrain(pidIntegral, -INTEGRAL_MAX, INTEGRAL_MAX);

  if (dt > 0) {
    pidDerivative = (pidError - pidPrevError) / dt;
  } else {
    pidDerivative = 0;
  }

  pidOutput = (KP * pidError) + (KI * pidIntegral) + (KD * pidDerivative);
  pidOutput = constrain(pidOutput, PID_OUTPUT_MIN, PID_OUTPUT_MAX);

  pidPrevError = pidError;

  return pidOutput;
}

// ======================== BUTTON HANDLER ========================

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
    case STATE_IDLE:
    case STATE_STOPPED:
      targetAngle  = currentAngle;
      pidIntegral  = 0;
      pidPrevError = 0;
      encLeftTicks  = 0;
      encRightTicks = 0;
      botState = STATE_RUNNING;
      Serial.println(F(">> RUNNING — heading locked"));
      Serial.print(F("   Target: "));
      Serial.println(targetAngle, 2);
      break;

    case STATE_RUNNING:
      stopMotors();
      botState = STATE_STOPPED;
      Serial.println(F(">> STOPPED by button"));
      break;

    default:
      break;
  }
}

// ======================== DEBUG OUTPUT ========================

void printDebug()
{
  printCounter++;
  if (printCounter < DEBUG_PRINT_INTERVAL) return;
  printCounter = 0;

  noInterrupts();
  long lTicks = encLeftTicks;
  long rTicks = encRightTicks;
  interrupts();

  Serial.print(F("A:"));
  Serial.print(currentAngle, 2);
  Serial.print(F(" T:"));
  Serial.print(targetAngle, 2);
  Serial.print(F(" E:"));
  Serial.print(pidError, 2);
  Serial.print(F(" PID:"));
  Serial.print(pidOutput, 1);
  Serial.print(F(" EL:"));
  Serial.print(lTicks);
  Serial.print(F(" ER:"));
  Serial.println(rTicks);
}

// ======================== SETUP ========================

void setup()
{
  Serial.begin(115200);
  Serial.println(F("\n=== MPU HEADING CORRECTION v1.0 ==="));

  // ---- I²C & MPU6050 ----
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
  digitalWrite(STBY, HIGH);

  // ---- Encoder pins ----
  pinMode(ENC_LEFT_A,  INPUT_PULLUP);
  pinMode(ENC_LEFT_B,  INPUT_PULLUP);
  pinMode(ENC_RIGHT_A, INPUT_PULLUP);
  pinMode(ENC_RIGHT_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_LEFT_A),  encLeftISR,  RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_RIGHT_A), encRightISR, RISING);

  // ---- Button ----
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // ---- Stop motors at boot ----
  stopMotors();

  // ---- Gyro calibration (up to 3 attempts) ----
  botState = STATE_CALIBRATING;
  bool calibrated = false;
  for (int attempt = 1; attempt <= 3; attempt++) {
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

// ======================== MAIN LOOP ========================

void loop()
{
  handleButton();

  unsigned long now = micros();
  if ((now - prevTimeMicros) < LOOP_INTERVAL_US) return;

  updateAngle();

  switch (botState)
  {
    case STATE_RUNNING:
    {
      float correction = computePID();

      int leftSpeed  = BASE_SPEED + (int)correction;
      int rightSpeed = BASE_SPEED - (int)correction;

      leftSpeed  = constrain(leftSpeed,  MIN_SPEED, MAX_SPEED);
      rightSpeed = constrain(rightSpeed, MIN_SPEED, MAX_SPEED);

      setMotors(leftSpeed, rightSpeed);

      printDebug();
      break;
    }

    case STATE_IDLE:
    case STATE_STOPPED:
    case STATE_CALIBRATING:
    default:
      break;
  }
}