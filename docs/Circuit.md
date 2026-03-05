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
                    ┌─────────────┐   │   ┌───►────────────────────────┐
                    │  L7805CV    │   │   │       TB6612FNG            │
                    │  Regulator  │   │   │                            │
                    │             │   │   │  VM ◄── Battery 11.1V     │
                    │ IN ◄── Bat  │   │   │  VCC ◄── 5V               │
                    │ OUT ─► 5V   │   │   │  GND ◄── GND              │
                    │ GND ─► GND  │   │   │                            │
                    │             │   │   │  STBY──► D13              │
                    └─────────────┘   │   │                            │
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
         Enc R-A ──►│ D2            D13 │──► STBY
         Enc L-A ──►│ D3            D12 │──► AIN1 (Right dir)
      Push Btn  ──►│ D4            D11 │──► AIN2 (Right dir)
     Right PWM  ◄──│ D5            D10 │──► BIN1 (Left dir)
      Left PWM  ◄──│ D6             D9 │──► BIN2 (Left dir)
         Enc R-B ──►│ D7             D8 │◄── Enc L-B
                    │                     │
     Left Echo  ──►│ A0             A7 │◄── Right US Echo (⚠️ analog)
     Left Trig  ◄──│ A1             A6 │◄── Right US Trig (⚠️ analog)
    Front Trig  ◄──│ A2             5V │──► VCC bus
    Front Echo  ──►│ A3            GND │──► GND bus
    MPU6050 SDA ◄─►│ A4            VIN │◄── (not used — powered via 5V pin)
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
    │ Channel A │──────│ D3 (INT1) │       │ Channel A │──────│ D2 (INT0) │
    │ Channel B │──────│ D8        │       │ Channel B │──────│ D7        │
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
    │ STBY         │───────────────────│ D13       │  (HIGH = active)
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
    │ Trig      │───────│ A2        │       │ Trig      │───────│ A1  │
    │ Echo      │───────│ A3        │       │ Echo      │───────│ A0  │
    │ GND       │───────│ GND       │───────│ GND       │
    └───────────┘       └───────────┘       └───────────┘

    Right HC-SR04
    ┌───────────┐
    │ VCC       │──► 5V
    │ Trig      │──► A6 (⚠️ analog-only pin)
    │ Echo      │──► A7 (⚠️ analog-only pin — use analogRead threshold)
    │ GND       │──► GND
    └───────────┘
```

---

## L7805CV Voltage Regulator

```
    3S LiPo (11.1V)
        │
    ┌───▼────────┐
    │ L7805CV    │
    │   IN  OUT  │───► 5V (to all logic devices)
    │   GND      │───► GND
    └────────────┘

    Not connected to any Arduino pin.
    Provides regulated 5V to:
      • Arduino Nano (5V pin)
      • TB6612 VCC
      • MPU6050
      • HC-SR04 sensors
      • Encoders
```

---

## Push Button Wiring

```
    Arduino Nano D4 ──┬── Tactile Button ──┬── GND
                       │                    │
                       └── Uses INPUT_PULLUP (internal pull-up resistor)
                           Press = LOW, Release = HIGH
                           Software debounce: 50 ms
```

No external resistor needed. The button is used to start/stop the robot.

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
         └──► L7805CV Regulator → 5V output
                   │
                   ├──► Arduino Nano (5V pin)
                   ├──► TB6612 VCC (logic power)
                   ├──► MPU6050 VCC
                   ├──► 3× HC-SR04 VCC
                   ├──► 2× Encoder VCC
                   └──► Push button (via INPUT_PULLUP)
```

---

## ⚠️ Important Notes

1. **A6/A7 limitation**: Analog-only pins — right ultrasonic uses `analogRead()` threshold instead of `pulseIn()`
2. **STBY must be HIGH** before motors will respond — set in `setup()`
3. **Encoder interrupts** only work on D2 (INT0) and D3 (INT1) on Nano
4. **Do NOT power servos/motors from Nano 5V** — use TB6612 VM from battery
5. **MPU6050 AD0 to GND** sets I²C address to 0x68 (default)
6. **Decoupling capacitor** (100nF) recommended near MPU6050 VCC for noise reduction
