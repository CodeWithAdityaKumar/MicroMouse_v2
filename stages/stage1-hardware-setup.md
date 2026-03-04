# Stage 1 — Hardware Setup (Foundation)

Before coding algorithms, your robot must **move reliably**.

### Step 1: Connect Components

Typical connection layout:

**Motor Driver → Arduino**

| Motor Driver | Arduino |
| ------------ | ------- |
| PWMA         | D5      |
| AIN1         | D12     |
| AIN2         | D11     |
| PWMB         | D6      |
| BIN1         | D10     |
| BIN2         | D9      |
| STBY         | D8      |

**Encoders**

| Encoder | Arduino |
| ------- | ------- |
| Left A  | D2      |
| Left B  | D4      |
| Right A | D3      |
| Right B | D7      |

**MPU6050**

| MPU | Arduino |
| --- | ------- |
| VCC | 5V      |
| GND | GND     |
| SDA | A4      |
| SCL | A5      |

**Ultrasonic Sensors**

| Sensor | Arduino |
| ------ | ------- |
| Front  | A0/A1   |
| Left   | A2/A3   |
| Right  | A6/A7   |

**Voltage Divider**

Battery → divider → **A6/A7**

---

### Step 2: Test Each Component Individually

Create **separate test programs**

Test in this order:

1️⃣ Motor forward/backward
2️⃣ Encoder count
3️⃣ MPU yaw angle
4️⃣ Ultrasonic distance
5️⃣ Battery voltage

⚠️ Do NOT combine everything yet.
