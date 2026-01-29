# 🔌 Wiring Table – RFID + Keypad Smart Door Lock (Arduino Uno)

This document outlines the **pin connections** used in the project.
All wiring is based on the **Arduino Uno**, and pin allocation reflects deliberate design choices made to fit within its GPIO limitations.

---

## 🧠 Notes Before Wiring

* Arduino Uno logic level: **5V**
* Buttons use **INPUT_PULLUP** (pressed = LOW)
* Common **GND** must be shared across all modules
* A 4×4 keypad is used physically, but **only a 2×2 section is connected**

---

## 🔲 Arduino Uno Pin Allocation Summary

| Arduino Pin | Connected Module | Purpose              |
| ----------- | ---------------- | -------------------- |
| D2–D7       | LCD              | Data & control lines |
| D8          | Servo            | Door lock control    |
| D9          | RFID             | Reset (RST)          |
| D10         | RFID             | Slave Select (SS)    |
| D11         | RFID             | SPI MOSI             |
| D12         | RFID             | SPI MISO             |
| D13         | RFID             | SPI SCK              |
| A0–A3       | Keypad           | Rows & columns       |
| A4          | Push Button      | Enter PIN mode       |
| A5          | Push Button      | Update time/date     |
| 5V          | All modules      | Power                |
| GND         | All modules      | Ground               |

---

## 📟 LCD (16×2) Connections

| LCD Pin | Arduino Uno Pin          |
| ------- | ------------------------ |
| RS      | D7                       |
| E       | D6                       |
| D4      | D5                       |
| D5      | D4                       |
| D6      | D3                       |
| D7      | D2                       |
| VSS     | GND                      |
| VDD     | 5V                       |
| VO      | Potentiometer (contrast) |
| A       | 5V (via resistor)        |
| K       | GND                      |

---

## 🪪 RFID Module (MFRC522)

| RFID Pin | Arduino Uno Pin |
| -------- | --------------- |
| SDA (SS) | D10             |
| SCK      | D13             |
| MOSI     | D11             |
| MISO     | D12             |
| RST      | D9              |
| VCC      | 3.3V ⚠️         |
| GND      | GND             |

> ⚠️ **Important:** MFRC522 operates at **3.3V**, not 5V.

---

## 🔢 Keypad (4×4 Used as 2×2)

Only a **subset of the keypad** is connected to conserve GPIO pins.

| Keypad Pin | Arduino Uno Pin |
| ---------- | --------------- |
| Row 1      | A0              |
| Row 2      | A1              |
| Column 1   | A2              |
| Column 2   | A3              |

📌 Remaining rows and columns are **intentionally unused**.

---

## 🔘 Push Buttons

| Button Function               | Arduino Pin | Wiring       |
| ----------------------------- | ----------- | ------------ |
| MODE Button (PIN input)       | A4          | Button → GND |
| INC Button (Update time/date) | A5          | Button → GND |

> Internal pull-up resistors are enabled in software.

---

## ⚙️ Servo Motor

| Servo Wire | Arduino Uno                       |
| ---------- | --------------------------------- |
| Signal     | D8                                |
| VCC        | 5V (or external 5–6V recommended) |
| GND        | GND                               |

⚠️ **Note:** For stability, an external power source is recommended for the servo if the system resets unexpectedly.

---

## 🔋 Power Distribution

| Source       | Modules Powered                          |
| ------------ | ---------------------------------------- |
| Arduino 5V   | LCD, Keypad, Buttons, Servo (light load) |
| Arduino 3.3V | RFID Module                              |
| GND          | Common ground for all components         |

---

## 🧩 Design Consideration

This wiring layout reflects intentional **Arduino Uno–specific trade-offs**, especially in keypad usage and GPIO allocation.
The system is fully functional while remaining **scalable** for expansion using:

* Arduino Mega
* I/O expanders (PCF8574)
* Shift registers

---
