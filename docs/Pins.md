# 📌 Micromouse v2 — Pin Mapping

> **MCU**: Arduino Nano (ATmega328P, 5V, 16 MHz)

---

## Complete Pin Assignment Table

| Arduino Pin |      Type      | Connected To       | Function                           |
| :---------: | :------------: | ------------------ | ---------------------------------- |
|   **D2**    | Digital (INT0) | Left Encoder A     | Hardware interrupt — tick counting |
|   **D3**    | Digital (INT1) | Right Encoder A    | Hardware interrupt — tick counting |
|   **D4**    |    Digital     | Push Button        | Start/stop (INPUT_PULLUP)          |
|   **D5**    |      PWM       | TB6612 PWMA        | Left motor speed                   |
|   **D6**    |      PWM       | TB6612 PWMB        | Right motor speed                  |
|   **D7**    |    Digital     | Left Encoder B     | Direction detection                |
|   **D8**    |    Digital     | Right Encoder B    | Direction detection                |
|   **D9**    |    Digital     | TB6612 BIN2        | Right motor direction              |
|   **D10**   |    Digital     | TB6612 BIN1        | Right motor direction              |
|   **D11**   |    Digital     | TB6612 AIN2        | Left motor direction               |
|   **D12**   |    Digital     | TB6612 AIN1        | Left motor direction               |
|   **D13**   |    Digital     | TB6612 STBY        | Standby (HIGH = active)            |
|   **A0**    | Analog/Digital | Left US Echo       | Ultrasonic echo                    |
|   **A1**    | Analog/Digital | Right US Echo      | Ultrasonic echo                    |
|   **A2**    | Analog/Digital | **Shared US Trig** | Trigger for all 3 sensors          |
|   **A3**    | Analog/Digital | Front US Echo      | Ultrasonic echo                    |
|   **A4**    |    I²C SDA     | MPU6050 SDA        | IMU data line                      |
|   **A5**    |    I²C SCL     | MPU6050 SCL        | IMU clock line                     |
|   **A6**    | ⚠️ Analog ONLY | _Free_             | Available                          |
|   **A7**    | ⚠️ Analog ONLY | _Free_             | Available                          |

---

## By Component

### Motor Driver (TB6612FNG)

```
Left Motor               Right Motor
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
Channel A → D2 (INT0)    Channel A → D3 (INT1)
Channel B → D7           Channel B → D8
VCC       → 5V           VCC       → 5V
GND       → GND          GND       → GND

Tick direction:
  Left motor forward  → +ve ticks
  Right motor forward → +ve ticks
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

### Ultrasonic Sensors (HC-SR04) — Shared Trigger

```
All 3 sensor Trig pins wired together → A2 (shared trigger)

Front Echo → A3
Left Echo  → A0
Right Echo → A1

VCC (all 3) → 5V
GND (all 3) → GND

⚠️ Must read sensors SEQUENTIALLY (30ms delay between reads)
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

| Status      | Count | Pins          |
| ----------- | :---: | ------------- |
| Used        |  14   | D2–D13, A0–A3 |
| I²C         |   2   | A4, A5        |
| Analog-only |   2   | A6, A7 (free) |
| **Free**    | **2** | A6, A7        |
