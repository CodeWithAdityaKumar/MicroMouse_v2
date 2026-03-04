# 🔌 Micromouse v2 — Circuit Diagram

> Text-based wiring diagram for the complete Micromouse v2 robot.

---

## Full System Wiring

```
                          ┌─────────────────────┐
                          │     3S LiPo         │
                          │     11.1V           │
                          └──┬──────┬───────┬───┘
                             │      │       │
                             │      │       │
                    ┌────────▼──┐   │   ┌───▼────────────────────────┐
                    │ Voltage   │   │   │       TB6612FNG            │
                    │ Divider   │   │   │                            │
                    │           │   │   │  VM ◄── Battery 11.1V     │
                    │ R1=30kΩ   │   │   │  VCC ◄── 5V               │
                    │ R2=10kΩ   │   │   │  GND ◄── GND              │
                    │           │   │   │                            │
                    │ OUT──►A6  │   │   │  STBY──► D8               │
                    └───────────┘   │   │                            │
                                    │   │  ┌─RIGHT MOTOR─┐          │
                          ┌─────────▼─┐ │  │ PWMA──► D5  │          │
                          │  Arduino  │ │  │ AIN1──► D12  │          │
                          │   Nano    │ │  │ AIN2──► D11  │          │
                          │          │ │  │ MOTORA──►[M1] │          │
                          │  VIN◄──┘  │ │  └──────────────┘          │
                          │           │ │                            │
                          │           │ │  ┌─LEFT MOTOR──┐          │
                          │           │ │  │ PWMB──► D6  │          │
                          │           │ │  │ BIN1──► D10  │          │
                          │           │ │  │ BIN2──► D9   │          │
                          │           │ │  │ MOTORB──►[M2]│          │
                          │           │ │  └──────────────┘          │
                          │           │ └────────────────────────────┘
                          │           │
                          └───────────┘
```

---

## Arduino Nano Pin-Level Wiring

```
                    ┌─────────────────────┐
                    │     ARDUINO NANO    │
                    │                     │
         Enc L-A ──►│ D2            D13 │──► (free / LED)
         Enc R-A ──►│ D3            D12 │──► AIN1 (Right dir)
         Enc L-B ──►│ D4            D11 │──► AIN2 (Right dir)
     Right PWM  ◄──│ D5            D10 │──► BIN1 (Left dir)
      Left PWM  ◄──│ D6             D9 │──► BIN2 (Left dir)
         Enc R-B ──►│ D7             D8 │──► STBY
                    │                     │
    Front Trig  ◄──│ A0             A7 │◄── ⚠️ Analog only
    Front Echo  ──►│ A1             A6 │◄── Battery / ⚠️ Analog only
     Left Trig  ◄──│ A2             5V │──► VCC bus
     Left Echo  ──►│ A3            GND │──► GND bus
    MPU6050 SDA ◄─►│ A4            VIN │◄── Battery 11.1V
    MPU6050 SCL ◄─►│ A5                │
                    └─────────────────────┘
```

---

## MPU6050 Wiring

```
    MPU6050            Arduino Nano
    ┌───────┐          ┌───────────┐
    │ VCC   │──────────│ 5V        │
    │ GND   │──────────│ GND       │
    │ SDA   │──────────│ A4 (SDA)  │
    │ SCL   │──────────│ A5 (SCL)  │
    │ AD0   │──────────│ GND       │  (I²C address = 0x68)
    │ INT   │          │           │  (not connected)
    └───────┘          └───────────┘
```

---

## Encoder Wiring

```
    Left Encoder       Arduino Nano         Right Encoder
    ┌───────────┐      ┌───────────┐       ┌───────────┐
    │ Channel A │──────│ D2 (INT0) │       │ Channel A │──────│ D3 (INT1) │
    │ Channel B │──────│ D4        │       │ Channel B │──────│ D7        │
    │ VCC       │──────│ 5V        │       │ VCC       │──────│ 5V        │
    │ GND       │──────│ GND       │       │ GND       │──────│ GND       │
    └───────────┘      └───────────┘       └───────────┘
```

---

## TB6612FNG Motor Driver Wiring

```
    TB6612FNG                          Arduino Nano
    ┌──────────────┐                   ┌───────────┐
    │ VM           │◄── Battery 11.1V  │           │
    │ VCC          │◄── 5V             │           │
    │ GND          │◄── GND            │           │
    │              │                   │           │
    │ STBY         │───────────────────│ D8        │  (HIGH = active)
    │              │                   │           │
    │ PWMA (Right) │───────────────────│ D5  (PWM) │
    │ AIN1         │───────────────────│ D12       │
    │ AIN2         │───────────────────│ D11       │
    │ MOTORA+      │──► Right Motor +  │           │
    │ MOTORA-      │──► Right Motor -  │           │
    │              │                   │           │
    │ PWMB (Left)  │───────────────────│ D6  (PWM) │
    │ BIN1         │───────────────────│ D10       │
    │ BIN2         │───────────────────│ D9        │
    │ MOTORB+      │──► Left Motor +   │           │
    │ MOTORB-      │──► Left Motor -   │           │
    └──────────────┘                   └───────────┘
```

---

## Ultrasonic Sensor Wiring

```
    Front HC-SR04       Arduino Nano        Left HC-SR04
    ┌───────────┐       ┌───────────┐       ┌───────────┐
    │ VCC       │───────│ 5V        │───────│ VCC       │
    │ Trig      │───────│ A0        │       │ Trig      │───────│ A2  │
    │ Echo      │───────│ A1        │       │ Echo      │───────│ A3  │
    │ GND       │───────│ GND       │───────│ GND       │
    └───────────┘       └───────────┘       └───────────┘

    Right HC-SR04
    ┌───────────┐
    │ VCC       │──► 5V
    │ Trig      │──► ⚠️ Needs digital pin (D13 or reassign)
    │ Echo      │──► ⚠️ Needs digital pin (D8 if STBY hardwired)
    │ GND       │──► GND
    └───────────┘
```

---

## Voltage Divider (Battery Monitor)

```
    Battery+  ────┐
                  │
               ┌──┴──┐
               │ R1   │  30kΩ
               └──┬──┘
                  │
                  ├──────────► A6 (Arduino Nano)
                  │
               ┌──┴──┐
               │ R2   │  10kΩ
               └──┬──┘
                  │
    GND ──────────┘

    Divider ratio = (R1 + R2) / R2 = 4.0
    Max input to A6 = 11.1V / 4.0 = 2.775V (safe for 5V ADC)
```

---

## Sensor Placement (Top View)

```
              FRONT ULTRASONIC
                    ↑
                ┌────────┐
                │        │
    LEFT US ◄── │  ROBOT │ ──► RIGHT US
                │        │
                │ [M1]  [M2] │
                └────────┘
              Left    Right
              Motor   Motor

    Sensor height: 2–3 cm from floor
    Orientation: perpendicular to wall face
```

---

## Power Distribution

```
    3S LiPo 11.1V
         │
         ├──► TB6612 VM (motor power, up to 13.5V)
         │
         ├──► Voltage Divider → A6 (battery monitoring)
         │
         └──► Arduino Nano VIN
                   │
                   └──► 5V regulator (onboard)
                          │
                          ├──► TB6612 VCC (logic power)
                          ├──► MPU6050 VCC
                          ├──► 3× HC-SR04 VCC
                          └──► 2× Encoder VCC
```

---

## ⚠️ Important Notes

1. **A6/A7 limitation**: Cannot use `pulseIn()` — reassign right ultrasonic to digital pins
2. **STBY must be HIGH** before motors will respond — set in `setup()`
3. **Encoder interrupts** only work on D2 (INT0) and D3 (INT1) on Nano
4. **Do NOT power servos/motors from Nano 5V** — use TB6612 VM from battery
5. **MPU6050 AD0 to GND** sets I²C address to 0x68 (default)
6. **Decoupling capacitor** (100nF) recommended near MPU6050 VCC for noise reduction
