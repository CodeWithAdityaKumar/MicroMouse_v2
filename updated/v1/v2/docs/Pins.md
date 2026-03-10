# Micromouse — Pin Mapping (Arduino Nano)

## Motor Driver (TB6612FNG)

| Pin  | Arduino Nano | Type    | Purpose                         |
| ---- | ------------ | ------- | ------------------------------- |
| PWMA | **D5**       | PWM     | Left motor speed                |
| AIN1 | **D12**      | Digital | Left motor direction A          |
| AIN2 | **D11**      | Digital | Left motor direction B          |
| PWMB | **D6**       | PWM     | Right motor speed               |
| BIN1 | **D10**      | Digital | Right motor direction A         |
| BIN2 | **D9**       | Digital | Right motor direction B         |
| STBY | **D13**      | Digital | Standby (HIGH = motors enabled) |

## IMU (MPU6050)

| Pin | Arduino Nano | Type  | Purpose    |
| --- | ------------ | ----- | ---------- |
| SDA | **A4**       | I²C   | Data line  |
| SCL | **A5**       | I²C   | Clock line |
| VCC | **5V**       | Power | 5V supply  |
| GND | **GND**      | Power | Ground     |

## Button

| Pin    | Arduino Nano | Type                   | Purpose                             |
| ------ | ------------ | ---------------------- | ----------------------------------- |
| Button | **D4**       | Digital (INPUT_PULLUP) | Heading reset / calibration trigger |

## Encoders (Interrupt-Driven)

| Pin             | Arduino Nano | Type           | Purpose                   |
| --------------- | ------------ | -------------- | ------------------------- |
| Left encoder A  | **D2**       | Digital (INT0) | Left wheel tick counting  |
| Left encoder B  | **D7**       | Digital        | Left wheel direction      |
| Right encoder A | **D3**       | Digital (INT1) | Right wheel tick counting |
| Right encoder B | **D8**       | Digital        | Right wheel direction     |

## Wall Sensors (3x GP2Y0A710K0F)

| Pin          | Arduino Nano | Type   | Purpose             |
| ------------ | ------------ | ------ | ------------------- |
| Left sensor  | **A0**       | Analog | Left wall distance  |
| Front sensor | **A1**       | Analog | Front wall distance |
| Right sensor | **A2**       | Analog | Right wall distance |

## Pin Usage Summary

```
Arduino Nano Pinout:
─────────────────────────────────
 D0  (RX)    → [FREE — Serial RX]
 D1  (TX)    → [FREE — Serial TX]
 D2  (INT0)  → ENC_LEFT_A (left encoder)
 D3  (INT1)  → ENC_RIGHT_A (right encoder)
 D4          → BUTTON_PIN
 D5  (PWM)   → PWMA (left motor speed)
 D6  (PWM)   → PWMB (right motor speed)
 D7          → ENC_LEFT_B (left encoder B)
 D8          → ENC_RIGHT_B (right encoder B)
 D9  (PWM)   → BIN2 (right motor dir B)
 D10 (PWM)   → BIN1 (right motor dir A)
 D11 (PWM)   → AIN2 (left motor dir B)
 D12         → AIN1 (left motor dir A)
 D13         → STBY (motor enable)
 A0          → IR Sensor LEFT
 A1          → IR Sensor FRONT
 A2          → IR Sensor RIGHT
 A3          → [FREE]
 A4 (SDA)    → MPU6050 SDA
 A5 (SCL)    → MPU6050 SCL
─────────────────────────────────
 Used: 18 pins
 Free: 2 pins (D0, D1, A3)
```
