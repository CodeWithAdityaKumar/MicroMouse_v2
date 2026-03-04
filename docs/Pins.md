# 📌 Micromouse v2 — Pin Mapping

> **MCU**: Arduino Nano (ATmega328P, 5V, 16 MHz)

---

## Complete Pin Assignment Table

| Arduino Pin |      Type      | Connected To       | Function                           |
| :---------: | :------------: | ------------------ | ---------------------------------- |
|   **D2**    | Digital (INT0) | Left Encoder A     | Hardware interrupt — tick counting |
|   **D3**    | Digital (INT1) | Right Encoder A    | Hardware interrupt — tick counting |
|   **D4**    |    Digital     | Left Encoder B     | Direction detection                |
|   **D5**    |      PWM       | TB6612 PWMA        | Right motor speed                  |
|   **D6**    |      PWM       | TB6612 PWMB        | Left motor speed                   |
|   **D7**    |    Digital     | Right Encoder B    | Direction detection                |
|   **D8**    |    Digital     | TB6612 STBY        | Standby (HIGH = active)            |
|   **D9**    |    Digital     | TB6612 BIN2        | Left motor direction               |
|   **D10**   |    Digital     | TB6612 BIN1        | Left motor direction               |
|   **D11**   |    Digital     | TB6612 AIN2        | Right motor direction              |
|   **D12**   |    Digital     | TB6612 AIN1        | Right motor direction              |
|   **D13**   |    Digital     | —                  | Free (onboard LED)                 |
|   **A0**    | Analog/Digital | Front US Trig      | Ultrasonic trigger                 |
|   **A1**    | Analog/Digital | Front US Echo      | Ultrasonic echo                    |
|   **A2**    | Analog/Digital | Left US Trig       | Ultrasonic trigger                 |
|   **A3**    | Analog/Digital | Left US Echo       | Ultrasonic echo                    |
|   **A4**    |    I²C SDA     | MPU6050 SDA        | IMU data line                      |
|   **A5**    |    I²C SCL     | MPU6050 SCL        | IMU clock line                     |
|   **A6**    | ⚠️ Analog ONLY | Right US / Battery | See note below                     |
|   **A7**    | ⚠️ Analog ONLY | Right US / Battery | See note below                     |

---

## By Component

### Motor Driver (TB6612FNG)

```
Right Motor              Left Motor
─────────────            ─────────────
PWMA  → D5  (speed)      PWMB  → D6  (speed)
AIN1  → D12 (dir)        BIN1  → D10 (dir)
AIN2  → D11 (dir)        BIN2  → D9  (dir)

STBY  → D8  (must be HIGH to run)
VM    → Battery (11.1V)
VCC   → 5V
GND   → GND
```

### Encoders

```
Left Encoder             Right Encoder
─────────────            ─────────────
Channel A → D2 (INT0)    Channel A → D3 (INT1)
Channel B → D4           Channel B → D7
VCC       → 5V           VCC       → 5V
GND       → GND          GND       → GND
```

### MPU6050

```
VCC → 5V
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
Trig → A0                Trig → A2                Trig → ⚠️ TBD
Echo → A1                Echo → A3                Echo → ⚠️ TBD
VCC  → 5V                VCC  → 5V                VCC  → 5V
GND  → GND               GND  → GND               GND  → GND
```

### Battery Monitor (Voltage Divider)

```
Battery+ → R1 (30kΩ) → junction → R2 (10kΩ) → GND
                         │
                         └──→ A6 or A7
```

---

## ⚠️ A6/A7 Pin Limitation

A6 and A7 on Arduino Nano are **analog input only**.

They **CANNOT** do:

- `digitalRead()`
- `digitalWrite()`
- `pulseIn()` (needed for HC-SR04 echo)

### Options:

1. **Reassign right ultrasonic** to D13 (Trig) + reuse another pin (Echo)
2. **Hardwire STBY to 5V** → frees D8 for right ultrasonic Echo
3. **Use only 2 ultrasonic sensors** (front + left) and turn to check right

---

## Free Pins

| Pin | Status         | Notes                                 |
| --- | -------------- | ------------------------------------- |
| D13 | Free           | Has onboard LED — usable as output    |
| A6  | Partially free | Analog-only, can read battery voltage |
| A7  | Partially free | Analog-only, can read battery voltage |

---

## Power Rails

```
3S LiPo (11.1V)
    │
    ├──→ TB6612 VM (motor power)
    ├──→ Voltage divider → A6 (monitoring)
    └──→ Arduino Nano VIN → onboard 5V regulator
              │
              ├──→ MPU6050 VCC
              ├──→ Encoders VCC
              └──→ HC-SR04 VCC (×3)
```
