/*
  RFID + Keypad Smart Door Lock (Non-Blocking) - Arduino Uno
  Author: Toluwanimi David

  Non-blocking refactor:
  - Uses a finite state machine (FSM)
  - Uses millis() timers instead of delay()
  - LCD scroll and time display continue while the system runs
  - RFID is responsive when in IDLE state
  - PIN entry and Update mode have timeouts without hard blocking delays

  Expected Serial update format:
    HH:MM DD/MM/YY
  Example:
    12:45 29/01/26
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

constexpr uint8_t MODE_BUTTON = A4; // Enter PIN mode
constexpr uint8_t INC_BUTTON  = A5; // Update time/date from PC

// LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// RFID + Servo
MFRC522 rfid(SS_PIN, RST_PIN);
Servo doorServo;

// ---------------------- Keypad (4x4 used as 2x2) ----------------------
constexpr byte ROWS = 2;
constexpr byte COLS = 2;

char keys[ROWS][COLS] = {
  {'1','2'},
  {'3','4'}
};

byte rowPins[ROWS] = {A0, A1};
byte colPins[COLS] = {A2, A3};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

/*
  NOTE:
  A 4x4 matrix keypad was used physically, but only a 2x2 section is implemented.
  This was a deliberate Arduino Uno trade-off due to limited GPIO after integrating:
  - RFID (SPI pins + SS/RST)
  - LCD (parallel pins)
  - Servo
  - Two control buttons
  The keypad mapping can be expanded on boards with more GPIO or via an I/O expander.
*/

// ---------------------- Behaviour Config ----------------------
constexpr uint8_t LCD_COLS = 16;
constexpr uint8_t LCD_ROWS = 2;

constexpr int SERVO_LOCK_ANGLE   = 0;
constexpr int SERVO_UNLOCK_ANGLE = 90;

constexpr unsigned long UNLOCK_TIME_MS     = 5000;
constexpr unsigned long DENY_TIME_MS       = 1500;
constexpr unsigned long PIN_TIMEOUT_MS     = 10000;
constexpr unsigned long UPDATE_TIMEOUT_MS  = 15000;

constexpr unsigned long SCROLL_INTERVAL_MS = 300;
constexpr unsigned long BUTTON_DEBOUNCE_MS = 180;

constexpr uint8_t PIN_LENGTH = 2;

// Credentials (examples)
const char* AUTH_PINS[] = {"12", "34"};
constexpr size_t AUTH_PINS_COUNT = sizeof(AUTH_PINS) / sizeof(AUTH_PINS[0]);

const char* AUTH_UIDS[] = {"F1061B06", "119D3206"};
constexpr size_t AUTH_UIDS_COUNT = sizeof(AUTH_UIDS) / sizeof(AUTH_UIDS[0]);

// ---------------------- Time/Date Storage ----------------------
char currentTimeStr[6] = "12:00";
char currentDateStr[9] = "01/01/26";

// Welcome message (kept in flash)
const char WELCOME_MSG[] PROGMEM = "Hello, welcome to the home of tech  ";
size_t scrollIndex = 0;

// ---------------------- System State ----------------------
enum SystemState : uint8_t {
  STATE_IDLE = 0,

  STATE_RFID_GRANTED,
  STATE_RFID_DENIED,

  STATE_PIN_ENTRY,
  STATE_PIN_GRANTED,
  STATE_PIN_DENIED,

  STATE_UPDATE_WAITING,
  STATE_UPDATE_OK,
  STATE_UPDATE_BAD,
  STATE_UPDATE_TIMEOUT
};

SystemState state = STATE_IDLE;

// Timing
unsigned long stateStartMs = 0;
unsigned long lastScrollMs = 0;
unsigned long lastBtnMs    = 0;

// Mode flags (used only for display behavior)
bool inPasswordMode = false;
bool inUpdateMode   = false;

// PIN entry buffer
char pinBuf[PIN_LENGTH + 1] = {0};
uint8_t pinCount = 0;

// Serial input buffer (non-blocking line reader)
char serialBuf[32];
uint8_t serialCount = 0;

// ---------------------- Helpers ----------------------
static bool isPressed(uint8_t pin) {
  return digitalRead(pin) == LOW;
}

static bool debounceOk() {
  unsigned long now = millis();
  if (now - lastBtnMs < BUTTON_DEBOUNCE_MS) return false;
  lastBtnMs = now;
  return true;
}

static void lockDoor() {
  doorServo.write(SERVO_LOCK_ANGLE);
}

static void unlockDoor() {
  doorServo.write(SERVO_UNLOCK_ANGLE);
}

static void lcdPrintPadded(uint8_t col, uint8_t row, const char* msg) {
  lcd.setCursor(col, row);
  for (uint8_t i = 0; i < LCD_COLS - col; i++) {
    char c = msg[i];
    lcd.print(c ? c : ' ');
  }
}

static void setState(SystemState next) {
  state = next;
  stateStartMs = millis();
}

static void displayTimeDate() {
  if (inPasswordMode || inUpdateMode) return;

  lcd.setCursor(0, 0);
  lcd.print(currentTimeStr);
  lcd.print(' ');
  lcd.print(currentDateStr);

  // Pad remainder
  const int used = 5 + 1 + 8;
  for (int i = used; i < LCD_COLS; i++) lcd.print(' ');
}

static void scrollWelcome() {
  if (inPasswordMode || inUpdateMode) return;

  unsigned long now = millis();
  if (now - lastScrollMs < SCROLL_INTERVAL_MS) return;
  lastScrollMs = now;

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

static bool pinAuthorized(const char* entered) {
  for (size_t i = 0; i < AUTH_PINS_COUNT; i++) {
    if (strcmp(entered, AUTH_PINS[i]) == 0) return true;
  }
  return false;
}

static void uidToHexUpper(const MFRC522::Uid& uid, char* out, size_t outSize) {
  const size_t needed = (uid.size * 2) + 1;
  if (outSize < needed) {
    if (outSize) out[0] = '\0';
    return;
  }

  static const char HEX[] = "0123456789ABCDEF";
  size_t p = 0;
  for (byte i = 0; i < uid.size; i++) {
    byte b = uid.uidByte[i];
    out[p++] = HEX[(b >> 4) & 0x0F];
    out[p++] = HEX[b & 0x0F];
  }
  out[p] = '\0';
}

static bool uidAuthorized(const char* uidHex) {
  for (size_t i = 0; i < AUTH_UIDS_COUNT; i++) {
    if (strcmp(uidHex, AUTH_UIDS[i]) == 0) return true;
  }
  return false;
}

static void showTwoLine(const char* l0, const char* l1) {
  lcd.clear();
  lcdPrintPadded(0, 0, l0);
  lcdPrintPadded(0, 1, l1);
}

// ---------------------- RFID Handler ----------------------
static void handleRFID() {
  if (state != STATE_IDLE) return;  // only scan when idle
  if (inPasswordMode || inUpdateMode) return;

  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  char uidHex[16];
  uidToHexUpper(rfid.uid, uidHex, sizeof(uidHex));

  if (uidAuthorized(uidHex)) {
    Serial.print(F("Access granted via RFID: "));
    Serial.println(uidHex);

    showTwoLine("ACCESS GRANTED", "RFID OK");
    unlockDoor();
    setState(STATE_RFID_GRANTED);
  } else {
    Serial.print(F("Access denied via RFID: "));
    Serial.println(uidHex);

    showTwoLine("ACCESS DENIED", "Unknown card");
    setState(STATE_RFID_DENIED);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// ---------------------- Button Handler ----------------------
static void handleButtons() {
  if (!debounceOk()) return;

  // MODE button -> PIN entry mode
  if (isPressed(MODE_BUTTON) && state == STATE_IDLE) {
    inPasswordMode = true;
    pinCount = 0;
    memset(pinBuf, 0, sizeof(pinBuf));

    showTwoLine("Password input:", "");
    lcd.setCursor(0, 1);

    setState(STATE_PIN_ENTRY);
    return;
  }

  // INC button -> Update mode
  if (isPressed(INC_BUTTON) && state == STATE_IDLE) {
    inUpdateMode = true;
    serialCount = 0;
    memset(serialBuf, 0, sizeof(serialBuf));

    showTwoLine("Update from PC", "HH:MM DD/MM/YY");
    setState(STATE_UPDATE_WAITING);
    return;
  }
}

// ---------------------- Keypad Handler ----------------------
static void handleKeypad() {
  if (state != STATE_PIN_ENTRY) return;

  // timeout
  if (millis() - stateStartMs >= PIN_TIMEOUT_MS) {
    Serial.println(F("PIN entry timeout"));
    showTwoLine("PIN TIMEOUT", "Returning...");
    setState(STATE_PIN_DENIED);
    return;
  }

  char key = keypad.getKey();
  if (!key) return;

  if (pinCount < PIN_LENGTH) {
    pinBuf[pinCount++] = key;
    lcd.print('*');
  }

  if (pinCount >= PIN_LENGTH) {
    if (pinAuthorized(pinBuf)) {
      Serial.print(F("Access Granted via PIN: "));
      Serial.println(pinBuf);

      showTwoLine("ACCESS GRANTED", "PIN OK");
      unlockDoor();
      setState(STATE_PIN_GRANTED);
    } else {
      Serial.print(F("Access Denied via PIN: "));
      Serial.println(pinBuf);

      showTwoLine("ACCESS DENIED", "Wrong PIN");
      setState(STATE_PIN_DENIED);
    }
  }
}

// ---------------------- Serial Update Handler ----------------------
static void handleSerialUpdate() {
  if (state != STATE_UPDATE_WAITING) return;

  // timeout
  if (millis() - stateStartMs >= UPDATE_TIMEOUT_MS) {
    Serial.println(F("Update mode timeout"));
    showTwoLine("No Serial input", "Update cancelled");
    setState(STATE_UPDATE_TIMEOUT);
    return;
  }

  // Non-blocking line reader:
  while (Serial.available()) {
    char c = (char)Serial.read();

    if (c == '\r') continue; // ignore CR
    if (c == '\n') {
      serialBuf[serialCount] = '\0';
      serialCount = 0;

      // Validate: "HH:MM DD/MM/YY" length 14 and space at index 5
      const size_t len = strlen(serialBuf);
      if (len >= 14 && serialBuf[5] == ' ') {
        // Copy time
        for (uint8_t i = 0; i < 5; i++) currentTimeStr[i] = serialBuf[i];
        currentTimeStr[5] = '\0';

        // Copy date
        for (uint8_t i = 0; i < 8; i++) currentDateStr[i] = serialBuf[6 + i];
        currentDateStr[8] = '\0';

        Serial.print(F("Time/Date updated: "));
        Serial.print(currentTimeStr);
        Serial.print(' ');
        Serial.println(currentDateStr);

        showTwoLine("UPDATED", "OK");
        setState(STATE_UPDATE_OK);
      } else {
        Serial.print(F("Bad update format: "));
        Serial.println(serialBuf);
        showTwoLine("BAD FORMAT", "HH:MM DD/MM/YY");
        setState(STATE_UPDATE_BAD);
      }

      return; // stop after processing a line
    }

    // Store into buffer if space remains
    if (serialCount < sizeof(serialBuf) - 1) {
      serialBuf[serialCount++] = c;
    } else {
      // overflow -> reset to avoid garbage
      serialCount = 0;
      memset(serialBuf, 0, sizeof(serialBuf));
      showTwoLine("SERIAL TOO LONG", "Try again");
      setState(STATE_UPDATE_BAD);
      return;
    }
  }
}

// ---------------------- State Resolver ----------------------
static void resolveState() {
  unsigned long elapsed = millis() - stateStartMs;

  switch (state) {
    case STATE_RFID_GRANTED:
      if (elapsed >= UNLOCK_TIME_MS) {
        lockDoor();
        lcd.clear();
        scrollIndex = 0;
        setState(STATE_IDLE);
      }
      break;

    case STATE_RFID_DENIED:
      if (elapsed >= DENY_TIME_MS) {
        lcd.clear();
        scrollIndex = 0;
        setState(STATE_IDLE);
      }
      break;

    case STATE_PIN_GRANTED:
      if (elapsed >= UNLOCK_TIME_MS) {
        lockDoor();
        inPasswordMode = false;
        lcd.clear();
        scrollIndex = 0;
        setState(STATE_IDLE);
      }
      break;

    case STATE_PIN_DENIED:
      // give deny message/timeouts a short display then go idle
      if (elapsed >= DENY_TIME_MS) {
        inPasswordMode = false;
        lcd.clear();
        scrollIndex = 0;
        setState(STATE_IDLE);
      }
      break;

    case STATE_UPDATE_OK:
    case STATE_UPDATE_BAD:
    case STATE_UPDATE_TIMEOUT:
      if (elapsed >= DENY_TIME_MS) {
        inUpdateMode = false;
        lcd.clear();
        scrollIndex = 0;
        setState(STATE_IDLE);
      }
      break;

    case STATE_PIN_ENTRY:
    case STATE_UPDATE_WAITING:
    case STATE_IDLE:
    default:
      break;
  }
}

// ---------------------- Setup / Loop ----------------------
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

  showTwoLine("SMART DOOR LOCK", "Non-blocking");
  stateStartMs = millis();
  // Let it show briefly without delay: we just transition after a moment
  setState(STATE_RFID_DENIED); // reuse short display timer
}

void loop() {
  // Background UI
  displayTimeDate();
  scrollWelcome();

  // Input handlers (non-blocking)
  handleRFID();
  handleButtons();
  handleKeypad();
  handleSerialUpdate();

  // Resolve timed states (unlock durations, deny screens, timeouts)
  resolveState();
}
