# 🔐 RFID + Keypad Smart Door Lock System (Arduino Uno)

A **smart access control system** built around the **Arduino Uno**, combining **RFID authentication**, **keypad-based PIN entry**, and a **servo-controlled door lock**, with real-time user feedback via a **16×2 LCD display**.

This project demonstrates practical embedded-systems design while working within **real hardware constraints**.

---

## ✨ Features

* RFID-based authentication using **MFRC522**
* PIN-based access via keypad
* Servo motor–controlled door locking mechanism
* 16×2 LCD with:

  * Time & date display
  * Scrolling welcome message
* Manual time/date update from PC via Serial Monitor
* Designed with **Arduino Uno limitations in mind**

---

## 🧠 Design Philosophy

This system was **written and majorly integrated for the Arduino Uno**, and all design decisions reflect that target platform.

Rather than overcomplicating the system with additional hardware, the project focuses on:

* Clear logic
* Hardware-aware decisions
* Expandability for future versions

---

## ⚙️ Hardware Constraints & Design Decisions

### Arduino Uno–Specific Implementation

The Arduino Uno provides a **limited number of GPIO pins**, which significantly influenced the system architecture.

One notable example is the **keypad integration**:

* A **4×4 matrix keypad** was used physically
* Only a **2×2 portion** of the keypad was implemented in software
* This decision was made to conserve GPIO pins after allocating pins for:

  * RFID module (SPI)
  * LCD display
  * Servo motor
  * Control buttons

Instead of introducing I/O expanders or shift registers, the keypad logic was **intentionally scoped** to a reduced matrix while still maintaining full authentication functionality.

This design choice is clearly reflected in the keypad configuration and scanning logic within the code.

> 💡 The keypad logic is easily scalable back to a full 4×4 matrix on boards with more GPIO pins (e.g., Arduino Mega) or with external pin expansion hardware.

---

## 🧩 Components Used

* Arduino Uno
* MFRC522 RFID Reader
* 16×2 LCD Display
* 4×4 Matrix Keypad (partially utilized)
* Servo Motor (Door Lock)
* Push Buttons (Mode & Update)
* Jumper Wires
* Breadboard / Enclosure

---

## 🧪 How to Use

### RFID Access

1. Power the system
2. Present an authorized RFID card
3. Servo unlocks the door for a few seconds, then locks automatically

### Keypad PIN Access

1. Press **MODE** button
2. Enter the PIN using the keypad
3. Access is granted or denied based on validation

### Update Time & Date

1. Press **INC** button
2. Send data via Serial Monitor in this format:

   ```
   HH:MM DD/MM/YY
   ```

   Example:

   ```
   12:45 29/01/26
   ```

---

## 📈 Future Improvements

* Full 4×4 keypad support using I/O expansion
* Non-blocking logic (`millis()`-based state machine)
* EEPROM storage for RFID cards and PINs
* RTC module for real-time clock
* Mobile or IoT integration

---

## 📸 Demo & Media

<img width="975" height="731" alt="image" src="https://github.com/user-attachments/assets/ee63a066-88ca-4a24-bf1f-17a531650c4b" />
<img width="975" height="1301" alt="image" src="https://github.com/user-attachments/assets/6296d8ad-026d-4380-beff-665e98f8fddf" />


---

## 🧑‍💻 Author

**Toluwanimi David**
Embedded Systems | Arduino | Robotics
📍 Nigeria
---

