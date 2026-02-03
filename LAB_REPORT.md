# 🔐 RFID + Keypad Smart Door Lock System

### **An Embedded Systems Laboratory Report**

**Name:** Toluwanimi David
**Course:** COE 411
**Institution:** Afe Babalola University
**Date:** 3/2/2026

---

## ABSTRACT

This project presents the design and implementation of a **smart door access control system** using the **Arduino Uno**. The system integrates **RFID-based authentication**, **keypad-based PIN entry**, and a **servo-controlled locking mechanism**, with user feedback provided via a **16×2 LCD display**.

The design emphasizes **hardware-aware decision-making**, particularly in managing the limited GPIO and memory resources of the Arduino Uno. A deliberate trade-off was made in the keypad implementation, where only a portion of a 4×4 keypad was utilized to fit within the available pin budget. Both **blocking** and **non-blocking** software architectures were explored, demonstrating scalability and responsiveness considerations in embedded system design.

---

## 1. INTRODUCTION

Access control systems are widely used in modern security applications, ranging from residential doors to industrial facilities. Traditional locking mechanisms often lack flexibility and multi-factor authentication.

This project aims to design a **low-cost, embedded access control system** that:

* Supports multiple authentication methods
* Provides clear user feedback
* Operates reliably on constrained hardware

The Arduino Uno was selected as the target platform to emphasize **practical embedded system constraints** and real-world design trade-offs.

---

## 2. SYSTEM OBJECTIVES

The primary objectives of this project are:

1. To design a smart door lock using RFID and keypad authentication
2. To implement secure access control using an Arduino Uno
3. To provide real-time system feedback via an LCD
4. To manage hardware limitations without introducing unnecessary components
5. To demonstrate both blocking and non-blocking embedded programming approaches

---

## 3. HARDWARE COMPONENTS USED

| Component                 | Description                    |
| ------------------------- | ------------------------------ |
| Arduino Uno               | Main microcontroller           |
| MFRC522 RFID Module       | Contactless authentication     |
| 16×2 LCD                  | User interface                 |
| 4×4 Matrix Keypad         | PIN input (partially utilized) |
| Servo Motor               | Door locking mechanism         |
| Push Buttons              | Mode selection                 |
| Breadboard & Jumper Wires | Prototyping                    |

---

## 4. SYSTEM DESIGN CONSIDERATIONS

### 4.1 Target Platform Constraints

The Arduino Uno provides:

* Limited GPIO pins
* Limited SRAM
* Fixed SPI pin assignments

These constraints influenced both **hardware wiring** and **software architecture**.

---

### 4.2 Keypad Design Trade-Off

Although a **4×4 matrix keypad** was used physically, only a **2×2 section** was implemented in software.

**Reason:**
After allocating pins for:

* RFID module (SPI communication)
* LCD display
* Servo motor
* Mode selection buttons

There were insufficient GPIO pins to support the full keypad without additional hardware.

**Decision:**
Rather than introducing I/O expanders or shift registers, the keypad logic was deliberately scoped to a reduced matrix. This preserved system simplicity while maintaining authentication functionality.

This design choice is reflected consistently in:

* Software logic
* Wiring layout
* System documentation

---

## 5. SOFTWARE ARCHITECTURE

### 5.1 Programming Language & Libraries

* Language: C/C++ (Arduino framework)
* Libraries used:

  * SPI
  * MFRC522
  * Servo
  * LiquidCrystal
  * Keypad

---

### 5.2 Blocking vs Non-Blocking Design

Two implementations were developed:

#### Blocking Version

* Uses `delay()` for timing
* Simple and easy to understand
* Suitable for demonstrations and initial testing

#### Non-Blocking Version

* Uses `millis()`-based timing
* Implemented using a **finite state machine**
* Maintains responsiveness during authentication and display updates
* More scalable and production-oriented

---

## 6. SYSTEM OPERATION

### 6.1 RFID Authentication

1. User presents an RFID card
2. UID is read and validated
3. If authorized, the servo unlocks the door temporarily
4. Door automatically re-locks after a fixed duration

---

### 6.2 Keypad Authentication

1. User presses the MODE button
2. PIN is entered via keypad
3. PIN is validated against stored credentials
4. Access is granted or denied accordingly

---

### 6.3 Time and Date Update

1. User presses the INC button
2. Time and date are sent from a PC via Serial Monitor
3. LCD display updates accordingly

---

## 7. RESULTS AND OBSERVATIONS

* The system reliably authenticated authorized RFID cards and PINs
* Unauthorized access attempts were correctly denied
* The servo locking mechanism functioned consistently
* The non-blocking version maintained responsiveness across all modes
* Hardware constraints were successfully managed without external expansion hardware

---

## 8. CHALLENGES ENCOUNTERED

* Limited GPIO availability on the Arduino Uno
* Managing multiple peripherals simultaneously
* Ensuring user feedback did not conflict with system timing
* Preventing system lock-ups in blocking code

These challenges were addressed through careful pin allocation, modular code design, and the introduction of a non-blocking architecture.

---

## 9. FUTURE IMPROVEMENTS

Potential extensions include:

* Full 4×4 keypad support using I/O expanders
* EEPROM-based storage for credentials
* Integration of a real-time clock (RTC)
* IoT-based remote access monitoring
* Power optimization and enclosure design

---

## 10. CONCLUSION

This project successfully demonstrates the design and implementation of a smart door lock system using the Arduino Uno. Beyond functionality, it highlights the importance of **engineering judgment**, particularly in making informed design trade-offs under hardware constraints.

The project serves as a strong example of embedded system design, combining hardware integration, software architecture, and clear technical documentation.

---

## REFERENCES

1. Arduino Uno Datasheet
2. MFRC522 RFID Module Documentation
3. Arduino SPI Library Reference
4. Arduino Servo Library Reference

---

## APPENDIX

* GitHub Repository: *(insert link)*
* Wiring Diagrams (Fritzing)
* Source Code (Blocking & Non-Blocking)
