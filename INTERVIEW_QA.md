## 1️⃣ “Can you briefly explain this project?”

**Answer:**

This project is a smart door access control system built on the Arduino Uno. It combines RFID authentication and keypad-based PIN entry to control a servo-driven locking mechanism, with user feedback provided through a 16×2 LCD display.

The main focus of the project was not just functionality, but designing a reliable system within the hardware constraints of the Arduino Uno, making deliberate trade-offs rather than adding unnecessary components.

---

## 2️⃣ “Why did you choose the Arduino Uno?”

**Answer:**

The Arduino Uno was chosen because it is widely used, resource-constrained, and well-suited for demonstrating core embedded systems principles.

Using the Uno forced careful GPIO planning, memory awareness, and structured code design. These constraints made it an ideal platform to showcase practical engineering decision-making rather than relying on more capable hardware.

---

## 3️⃣ “Why didn’t you use the full 4×4 keypad?”

**Answer:**

Although a 4×4 keypad was used physically, only a 2×2 portion was implemented in software. This was a deliberate design decision due to limited available GPIO pins on the Arduino Uno after integrating the RFID module (SPI), LCD, servo motor, and control buttons.

Instead of introducing additional hardware such as I/O expanders, the keypad logic was scoped down while maintaining full authentication functionality. The code and wiring were written in a way that allows easy expansion to a full 4×4 keypad on boards with more GPIO pins.

---

## 4️⃣ “How does the RFID authentication work?”

**Answer:**

The MFRC522 module communicates with the Arduino Uno using SPI. When a card is presented, the UID is read byte-by-byte, converted into an uppercase hexadecimal string, and compared against a predefined list of authorized UIDs.

If a match is found, access is granted and the servo unlocks the door temporarily before automatically re-locking.

---

## 5️⃣ “How is security handled in this system?”

**Answer:**

Security is handled at multiple levels:

* RFID authentication using unique card UIDs
* PIN-based authentication as a fallback method
* Automatic re-locking of the servo after a fixed interval
* Clear user feedback via the LCD to prevent ambiguous states

While this is a prototype, the structure supports future enhancements such as EEPROM-based credential storage or multi-factor authentication.

---

## 6️⃣ “Why did you use blocking delays instead of a non-blocking design?”

**Answer:**

Blocking delays were intentionally used to keep the logic clear and readable for a demonstration and lab-scale project. This approach simplifies timing behavior and makes system flow easier to trace during testing and review.

However, the system architecture can be refactored into a non-blocking, `millis()`-based state machine if responsiveness or multitasking becomes a priority.

---

## 7️⃣ “What were the main challenges you faced?”

**Answer:**

The main challenge was managing limited GPIO and memory resources while integrating multiple peripherals simultaneously. This required careful pin allocation, simplifying certain subsystems like the keypad, and keeping the code modular and readable.

Another challenge was ensuring consistent user feedback while preventing the system from entering unsafe or ambiguous states.

---

## 8️⃣ “What would you improve if you had more time or resources?”

**Answer:**

Possible improvements include:

* Expanding the keypad to full 4×4 using an I/O expander
* Refactoring the code into a non-blocking state machine
* Adding EEPROM-based credential management
* Integrating a real-time clock (RTC) module
* Introducing remote monitoring or IoT features

---

## 9️⃣ “What skills does this project demonstrate?”

**Answer:**

This project demonstrates:

* Embedded systems design under constraints
* SPI communication
* GPIO planning and trade-off analysis
* Modular code structuring
* Hardware-software integration
* Technical documentation and communication

---

## 🔟 “Why should we consider you for an internship?”

**Answer:**

This project shows that I can take a problem, design a working solution, and make informed engineering trade-offs based on hardware limitations. I focus not only on making things work, but on making design decisions that are logical, documented, and scalable.

That mindset is what I aim to bring into an internship or engineering team.
