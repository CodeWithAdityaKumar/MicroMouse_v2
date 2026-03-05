# 📌 Micromouse v2 — Pin Mapping

> **MCU**: Arduino Nano (ATmega328P, 5V, 16 MHz)

---

## Complete Pin Assignment Table

| Arduino Pin |      Type      | Connected To    | Function                           |
| :---------: | :------------: | --------------- | ---------------------------------- |
|   **D2**    | Digital (INT0) | Right Encoder A | Hardware interrupt — tick counting |
|   **D3**    | Digital (INT1) | Left Encoder A  | Hardware interrupt — tick counting |
|   **D4**    |    Digital     | Push Button     | Start/stop (INPUT_PULLUP)          |
|   **D5**    |      PWM       | TB6612 PWMA     | Right motor speed                  |
|   **D6**    |      PWM       | TB6612 PWMB     | Left motor speed                   |
|   **D7**    |    Digital     | Right Encoder B | Direction detection                |
|   **D8**    |    Digital     | Left Encoder B  | Direction detection                |
|   **D9**    |    Digital     | TB6612 BIN2     | Left motor direction               |
|   **D10**   |    Digital     | TB6612 BIN1     | Left motor direction               |
|   **D11**   |    Digital     | TB6612 AIN2     | Right motor direction              |
|   **D12**   |    Digital     | TB6612 AIN1     | Right motor direction              |
|   **D13**   |    Digital     | TB6612 STBY     | Standby (HIGH = active)            |
|   **A0**    | Analog/Digital | Left US Echo    | Ultrasonic echo                    |
|   **A1**    | Analog/Digital | Left US Trig    | Ultrasonic trigger                 |
|   **A2**    | Analog/Digital | Front US Trig   | Ultrasonic trigger                 |
|   **A3**    | Analog/Digital | Front US Echo   | Ultrasonic echo                    |
|   **A4**    |    I²C SDA     | MPU6050 SDA     | IMU data line                      |
|   **A5**    |    I²C SCL     | MPU6050 SCL     | IMU clock line                     |
|   **A6**    | ⚠️ Analog ONLY | Right US Trig   | Ultrasonic trigger (analog-only)   |
|   **A7**    | ⚠️ Analog ONLY | Right US Echo   | Ultrasonic echo (analog-only)      |

---

## By Component

### Motor Driver (TB6612FNG)

```
Right Motor              Left Motor
─────────────            ─────────────
PWMA  → D5  (speed)      PWMB  → D6  (speed)
AIN1  → D12 (dir)        BIN1  → D10 (dir)
AIN2  → D11 (dir)        BIN2  → D9  (dir)

STBY  → D13 (must be HIGH to run)
VM    → Battery (11.1V)
VCC   → 5V (from L7805CV)
GND   → GND
```

### Encoders

```
Left Encoder             Right Encoder
─────────────            ─────────────
Channel A → D3 (INT1)    Channel A → D2 (INT0)
Channel B → D8           Channel B → D7
VCC       → 5V           VCC       → 5V
GND       → GND          GND       → GND

Tick direction:
  Left motor forward  → +ve ticks
  Right motor forward → -ve ticks
```

### MPU6050

```
VCC → 5V (from L7805CV)
GND → GND
SDA → A4
SCL → A5
AD0 → GND (I²C address = 0x68)
INT → not connected
```

### Ultrasonic Sensors (HC-SR04)

```
Front                    Left                     Right
─────────────            ─────────────            ─────────────
Trig → A2                Trig → A1                Trig → A6 (⚠️ analog-only)
Echo → A3                Echo → A0                Echo → A7 (⚠️ analog-only)
VCC  → 5V                VCC  → 5V                VCC  → 5V
GND  → GND               GND  → GND               GND  → GND
```

### Push Button

```
D4 ──┬── Button ──┬── GND
     │            │
     └── (uses INPUT_PULLUP, no external resistor needed)
         Press = LOW, Release = HIGH
         Debounce: 50 ms
```

---

## ⚠️ A6/A7 Pin Limitation

A6 and A7 on Arduino Nano are **analog input only**.

They **CANNOT** do:

- `digitalRead()`
- `digitalWrite()`
- `pulseIn()` (needed for standard HC-SR04 echo)

The right ultrasonic sensor on A6/A7 requires `analogRead()` threshold detection instead of standard `pulseIn()`.

---

## Power Regulation

Using **L7805CV** linear voltage regulator — not connected to any Arduino pin.

```
3S LiPo (11.1V)
    │
    ├──→ TB6612 VM (motor power, up to 13.5V)
    │
    └──→ L7805CV Voltage Regulator → 5V output
              │
              ├──→ Arduino Nano 5V pin
              ├──→ TB6612 VCC (logic power)
              ├──→ MPU6050 VCC
              ├──→ 3× HC-SR04 VCC
              ├──→ 2× Encoder VCC
              └──→ Push button (via INPUT_PULLUP)
```

---

## Pin Usage Summary

| Status      | Count | Pins                             |
| ----------- | :---: | -------------------------------- |
| Used        |  14   | D2–D13, A0–A3                    |
| I²C         |   2   | A4, A5                           |
| Analog-only |   2   | A6 (Right Trig), A7 (Right Echo) |
| **Free**    | **0** | All pins allocated               |
