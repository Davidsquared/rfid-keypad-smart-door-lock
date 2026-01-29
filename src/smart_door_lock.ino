/*
  RFID + Keypad Smart Door Lock (Arduino Uno)

  Features:
  - 16x2 LCD: shows time/date on row 0 and a scrolling welcome message on row 1
  - RFID (MFRC522): authorized UID unlocks the servo briefly
  - 2x2 Keypad: authorized PIN unlocks the servo
  - Button A4: enter PIN mode
  - Button A5: update time/date over Serial ("HH:MM DD/MM/YY")

  Notes:
  - Uses INPUT_PULLUP on buttons (pressed = LOW)
  - This is a blocking-style demo sketch (uses delay). Good for a lab project.
*/

#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <LiquidCrystal.h>
#include <Keypad.h>

// ---------------------- Hardware Pins ----------------------
constexpr uint8_t RST_PIN     = 9;
constexpr uint8_t SS_PIN      = 10;
constexpr uint8_t SERVO_PIN   = 8;

constexpr uint8_t MODE_BUTTON = A4;   // Enter PIN mode
constexpr uint8_t INC_BUTTON  = A5;   // Update time/date from PC (Serial)

// LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// RFID + Servo
MFRC522 rfid(SS_PIN, RST_PIN);
Servo doorServo;

// ---------------------- Keypad (2x2) ----------------------
constexpr byte ROWS = 2;
constexpr byte COLS = 2;

char keys[ROWS][COLS] = {
  {'1','2'},
  {'3','4'}
};

byte rowPins[ROWS] = {A0, A1};
byte colPins[COLS] = {A2, A3};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ---------------------- Behaviour Config ----------------------
constexpr uint8_t LCD_COLS = 16;
constexpr uint8_t LCD_ROWS = 2;

constexpr int SERVO_LOCK_ANGLE   = 0;
constexpr int SERVO_UNLOCK_ANGLE = 90;

constexpr unsigned long UNLOCK_TIME_MS      = 5000;
constexpr unsigned long PIN_TIMEOUT_MS      = 10000;
constexpr unsigned long UPDATE_TIMEOUT_MS   = 15000;
constexpr unsigned long SCROLL_INTERVAL_MS  = 300;
constexpr unsigned long BUTTON_DEBOUNCE_MS  = 200;

constexpr uint8_t PIN_LENGTH = 2;

// Use fixed-size arrays so it looks deliberate and avoids dynamic allocation.
const char* AUTH_PINS[] = {"12", "34"};
constexpr size_t AUTH_PINS_COUNT = sizeof(AUTH_PINS) / sizeof(AUTH_PINS[0]);

// Store RFID UIDs in uppercase hex with no spaces (example: "F1061B06")
const char* AUTH_UIDS[] = {"F1061B06", "119D3206"};
constexpr size_t AUTH_UIDS_COUNT = sizeof(AUTH_UIDS) / sizeof(AUTH_UIDS[0]);

// ---------------------- UI / State ----------------------
char currentTime[6] = "12:00";     // "HH:MM"
char currentDate[9] = "01/01/26";  // "DD/MM/YY"

// Put the message in flash to save RAM. We copy small windows out while scrolling.
const char WELCOME_MSG[] PROGMEM = "Hello, welcome to the home of tech  ";

bool inPasswordMode = false;
bool inUpdateMode   = false;

size_t scrollIndex = 0;
unsigned long lastScrollMs = 0;

// ---------------------- Helpers ----------------------
static bool isButtonPressed(uint8_t pin) {
  return digitalRead(pin) == LOW;
}

static void lockDoor() {
  doorServo.write(SERVO_LOCK_ANGLE);
}

static void unlockDoorFor(unsigned long ms) {
  doorServo.write(SERVO_UNLOCK_ANGLE);
  delay(ms);
  doorServo.write(SERVO_LOCK_ANGLE);
}

static void showCenteredRow(uint8_t row, const char* msg) {
  lcd.setCursor(0, row);
  // Print exactly LCD_COLS chars to avoid leftover characters.
  for (uint8_t i = 0; i < LCD_COLS; i++) {
    char c = msg[i];
    lcd.print(c ? c : ' ');
  }
}

static void showStatus(const char* line0, const char* line1, unsigned long holdMs) {
  lcd.clear();
  showCenteredRow(0, line0);
  showCenteredRow(1, line1);
  delay(holdMs);
}

static void displayTimeDate() {
  if (inPasswordMode || inUpdateMode) return;

  lcd.setCursor(0, 0);

  // Print "HH:MM DD/MM/YY" and pad
  lcd.print(currentTime);
  lcd.print(' ');
  lcd.print(currentDate);

  // Pad remainder to clear old chars
  int used = 5 + 1 + 8;
  for (int i = used; i < LCD_COLS; i++) lcd.print(' ');
}

static void scrollWelcome() {
  if (inPasswordMode || inUpdateMode) return;

  const unsigned long now = millis();
  if (now - lastScrollMs < SCROLL_INTERVAL_MS) return;
  lastScrollMs = now;

  // Build a 16-char window from the PROGMEM message
  char window[LCD_COLS + 1];
  window[LCD_COLS] = '\0';

  const size_t msgLen = strlen_P(WELCOME_MSG);
  for (uint8_t i = 0; i < LCD_COLS; i++) {
    size_t idx = (scrollIndex + i) % msgLen;
    window[i] = pgm_read_byte_near(WELCOME_MSG + idx);
  }

  lcd.setCursor(0, 1);
  lcd.print(window);

  scrollIndex = (scrollIndex + 1) % msgLen;
}

static bool pinIsAuthorized(const char* entered) {
  for (size_t i = 0; i < AUTH_PINS_COUNT; i++) {
    if (strcmp(entered, AUTH_PINS[i]) == 0) return true;
  }
  return false;
}

static void uidToHexUpper(const MFRC522::Uid& uid, char* out, size_t outSize) {
  // Need 2 chars per byte + null terminator.
  // Example: 4 bytes => 8 chars + '\0'
  const size_t needed = (uid.size * 2) + 1;
  if (outSize < needed) {
    if (outSize > 0) out[0] = '\0';
    return;
  }

  size_t pos = 0;
  for (byte i = 0; i < uid.size; i++) {
    byte b = uid.uidByte[i];
    const char hex[] = "0123456789ABCDEF";
    out[pos++] = hex[(b >> 4) & 0x0F];
    out[pos++] = hex[b & 0x0F];
  }
  out[pos] = '\0';
}

static bool uidIsAuthorized(const char* uidHex) {
  for (size_t i = 0; i < AUTH_UIDS_COUNT; i++) {
    if (strcmp(uidHex, AUTH_UIDS[i]) == 0) return true;
  }
  return false;
}

// ---------------------- Modes ----------------------
static void runPasswordMode() {
  inPasswordMode = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Password input:");
  lcd.setCursor(0, 1);

  char entered[PIN_LENGTH + 1] = {0};
  uint8_t count = 0;

  const unsigned long start = millis();
  while (millis() - start < PIN_TIMEOUT_MS) {
    char key = keypad.getKey();
    if (!key) continue;

    if (count < PIN_LENGTH) {
      entered[count++] = key;
      lcd.print('*');
    }

    if (count >= PIN_LENGTH) {
      if (pinIsAuthorized(entered)) {
        Serial.print(F("Access Granted via PIN: "));
        Serial.println(entered);

        showStatus("ACCESS GRANTED", "Unlocking...", 600);
        unlockDoorFor(UNLOCK_TIME_MS);
      } else {
        Serial.print(F("Access Denied via PIN: "));
        Serial.println(entered);

        showStatus("ACCESS DENIED", "Try again", 1200);
      }
      break;
    }
  }

  inPasswordMode = false;
  lcd.clear();
  scrollIndex = 0;
}

static void runUpdateMode() {
  inUpdateMode = true;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Update from PC");
  lcd.setCursor(0, 1);
  lcd.print("HH:MM DD/MM/YY");

  // Expected line: "12:34 29/01/26"
  const unsigned long start = millis();
  while (millis() - start < UPDATE_TIMEOUT_MS) {
    if (!Serial.available()) continue;

    String data = Serial.readStringUntil('\n');
    data.trim();

    // Minimal validation: "HH:MM DD/MM/YY" => length 14 and a space at index 5
    if (data.length() >= 14 && data.charAt(5) == ' ') {
      // Copy time
      for (uint8_t i = 0; i < 5; i++) currentTime[i] = data[i];
      currentTime[5] = '\0';

      // Copy date
      for (uint8_t i = 0; i < 8; i++) currentDate[i] = data[6 + i];
      currentDate[8] = '\0';

      Serial.print(F("Time/Date updated: "));
      Serial.print(currentTime);
      Serial.print(' ');
      Serial.println(currentDate);

      showStatus("UPDATED", "OK", 1200);
      break;
    } else {
      showStatus("BAD FORMAT", "Use HH:MM DD/MM/YY", 1200);
      // continue waiting until timeout
    }
  }

  if (millis() - start >= UPDATE_TIMEOUT_MS) {
    showStatus("No Serial input", "Update cancelled", 1200);
  }

  inUpdateMode = false;
  lcd.clear();
  scrollIndex = 0;
}

static void checkRFID() {
  if (inPasswordMode || inUpdateMode) return;

  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  char uidHex[16]; // enough for up to 7 bytes UID (14 chars + null)
  uidToHexUpper(rfid.uid, uidHex, sizeof(uidHex));

  const bool ok = uidIsAuthorized(uidHex);

  if (ok) {
    Serial.print(F("Access granted via RFID: "));
    Serial.println(uidHex);

    showStatus("ACCESS GRANTED", "RFID OK", 600);
    unlockDoorFor(UNLOCK_TIME_MS);
  } else {
    Serial.print(F("Access denied via RFID: "));
    Serial.println(uidHex);

    showStatus("ACCESS DENIED", "Unknown card", 1500);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  lcd.clear();
  scrollIndex = 0;
}

// ---------------------- Arduino Setup / Loop ----------------------
void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  doorServo.attach(SERVO_PIN);
  lockDoor();

  pinMode(MODE_BUTTON, INPUT_PULLUP);
  pinMode(INC_BUTTON, INPUT_PULLUP);

  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.clear();

  showStatus("SMART DOOR LOCK", "Ready", 1000);
  lcd.clear();
}

void loop() {
  displayTimeDate();
  scrollWelcome();

  // Automatic RFID check
  checkRFID();

  // Button-driven modes
  if (isButtonPressed(MODE_BUTTON)) {
    delay(BUTTON_DEBOUNCE_MS);
    if (isButtonPressed(MODE_BUTTON)) runPasswordMode();
  }

  if (isButtonPressed(INC_BUTTON)) {
    delay(BUTTON_DEBOUNCE_MS);
    if (isButtonPressed(INC_BUTTON)) runUpdateMode();
  }
}
