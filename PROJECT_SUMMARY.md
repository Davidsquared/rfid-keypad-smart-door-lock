# 🔐 RFID + Keypad Smart Door Lock System

**Platform:** Arduino Uno
**Author:** Toluwanimi David

---

## 📌 Project Overview

This project is a **smart door access control system** built around the **Arduino Uno**, combining **RFID authentication**, **keypad-based PIN entry**, and a **servo-controlled locking mechanism**, with real-time user feedback provided through a **16×2 LCD display**.

The system was designed to demonstrate **practical embedded-systems engineering**, particularly how to make **intentional design trade-offs** when working under **hardware constraints**.

---

## 🎯 Problem Statement

Traditional door locking systems either lack flexibility or rely on a single authentication method. The goal of this project was to design a **low-cost, multi-factor access system** that:

* Supports **RFID-based access**
* Provides a **manual PIN fallback**
* Offers clear user feedback
* Operates reliably on limited hardware resources

---

## ⚙️ Target Platform & Constraints

The system was **written and majorly integrated for the Arduino Uno**, which has:

* Limited GPIO pins
* Limited RAM
* Fixed hardware SPI usage

These constraints directly influenced several architectural decisions in both **hardware wiring** and **software logic**.

---

## 🧠 Key Design Decisions

### 1. Keypad Configuration Trade-Off

Although a **4×4 matrix keypad** was used physically, only a **2×2 section** was implemented in software.

**Reason:**
After allocating pins for the RFID module (SPI), LCD, servo motor, and control buttons, there were not enough GPIO pins remaining on the Arduino Uno to support the full keypad without additional hardware.

**Decision:**
Rather than introducing I/O expanders or shift registers, the keypad logic was intentionally scoped to a reduced matrix while maintaining full authentication functionality.

This design choice is clearly reflected in:

* The keypad pin mapping
* The keypad scanning logic in code
* The wiring and schematic diagrams

---

### 2. Modular and Readable Code Structure

The codebase was structured to separate:

* Hardware configuration
* User interface logic (LCD, scrolling text)
* Authentication logic (RFID & PIN)
* Actuation logic (servo control)

This improves readability, maintainability, and scalability.

---

### 3. User Feedback & Safety

The system provides immediate visual feedback for:

* Access granted
* Access denied
* Mode switching
* Time/date updates

The servo automatically **re-locks after a fixed interval**, preventing accidental unlocked states.

---

## 🛠️ Technologies & Components Used

* Arduino Uno
* MFRC522 RFID Reader (SPI)
* 16×2 LCD Display (parallel mode)
* 4×4 Matrix Keypad (partially utilized)
* Servo Motor
* Push Buttons (INPUT_PULLUP configuration)
* Fritzing (schematics & wiring diagrams)

---

## 🧪 System Operation Summary

* **RFID Mode:**
  Authorized RFID card → door unlocks temporarily → auto-locks

* **Keypad Mode:**
  MODE button → PIN entry → validation → unlock or deny

* **Update Mode:**
  INC button → Serial input (`HH:MM DD/MM/YY`) → LCD update

---

## 📈 Possible Improvements & Extensions

* Full 4×4 keypad support using I/O expanders
* Non-blocking, `millis()`-based state machine
* EEPROM storage for RFID cards and PINs
* Real-Time Clock (RTC) module integration
* IoT or mobile app integration

---

## 🧑‍💻 Skills Demonstrated

* Embedded systems design
* Hardware-aware software architecture
* GPIO management under constraints
* SPI communication
* Documentation & technical communication
* Practical debugging and testing

---

## 🏁 Conclusion

This project demonstrates not just functionality, but **engineering judgment**—balancing system requirements against real hardware limitations. It reflects a practical understanding of embedded-system design, where making informed trade-offs is as important as writing working code.
