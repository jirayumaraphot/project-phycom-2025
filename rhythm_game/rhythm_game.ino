#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int buttonPins[2] = {7, 8};
const int LCD_COLS = 12;
const int LCD_ROWS = 2;

int notes[LCD_ROWS][LCD_COLS];
unsigned long lastStep = 0;
int stepInterval = 400; // ms per movement
int score = 0;

String feedbackTop = "";
String feedbackBottom = "";
unsigned long feedbackTimeTop = 0;
unsigned long feedbackTimeBottom = 0;

// 🎵 Note
byte noteChar[8] = {
  B00000,
  B01110,
  B10001,
  B10001,
  B10001,
  B10001,
  B01110,
  B00000
};

// ✴️ Hit zone (solid block)
byte hitZoneChar[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

// 💫 Hit effect (flash)
byte hitEffectChar[8] = {
  B00000,
  B01110,
  B11111,
  B11111,
  B11111,
  B11111,
  B01110,
  B00000
};

// 🟢 GO symbol
byte goChar[8] = {
  B00000,
  B01110,
  B10001,
  B10111,
  B10001,
  B01110,
  B00000,
  B00000
};

void setup() {
  lcd.init();
  lcd.backlight();

  lcd.createChar(0, noteChar);
  lcd.createChar(1, hitZoneChar);
  lcd.createChar(2, hitEffectChar);
  lcd.createChar(3, goChar);

  for (int i = 0; i < 2; i++) pinMode(buttonPins[i], INPUT_PULLUP);

  startCountdown();
  generatePattern();
  displayNotes();
}

void loop() {
  unsigned long now = millis();

  // Move notes
  if (now - lastStep >= stepInterval) {
    lastStep = now;
    shiftNotesLeft();
    displayNotes();
  }

  // Handle button presses
  handleButtons();

  // Clear top feedback
  if (feedbackTop != "" && millis() - feedbackTimeTop > 500) {
    feedbackTop = "";
    displayNotes();
  }

  // Clear bottom feedback
  if (feedbackBottom != "" && millis() - feedbackTimeBottom > 500) {
    feedbackBottom = "";
    displayNotes();
  }

  delay(50);
}

// -------------------- FUNCTIONS --------------------

void startCountdown() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Get Ready!");

  for (int i = 3; i >= 1; i--) {
    lcd.setCursor(7, 1);
    lcd.print(i);
    delay(700);
    lcd.setCursor(7, 1);
    lcd.print(" ");
  }

  for (int blink = 0; blink < 3; blink++) {
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("GO!");
    lcd.setCursor(7, 1);
    lcd.write(byte(3));
    delay(250);
    lcd.clear();
    delay(200);
  }
  lcd.clear();
}

void shiftNotesLeft() {
  for (int r = 0; r < LCD_ROWS; r++) {
    // Move note into hit zone
    if (notes[r][1] == 1) {
      notes[r][0] = 2; // note entering hit zone (flash)
    } else if (notes[r][0] == 2) {
      notes[r][0] = 0; // clear hit zone flash
    }

    // Shift all notes left (excluding hit zone column 0)
    for (int c = 1; c < LCD_COLS - 1; c++) {
      notes[r][c] = notes[r][c + 1];
    }

    // Spawn new note randomly at far right
    notes[r][LCD_COLS - 1] = (random(0, 10) < 3) ? 1 : 0;
  }
}


void displayNotes() {
  lcd.clear();

  for (int r = 0; r < LCD_ROWS; r++) {
    for (int c = 0; c < LCD_COLS; c++) {
      lcd.setCursor(c, r);

      if (c == 0) { // Hit zone column
        if (notes[r][0] == 2)
          lcd.write(byte(2)); // flash effect
        else
          lcd.write(byte(1)); // solid hit zone
      } else if (notes[r][c] == 1) {
        lcd.write(byte(0));   // moving note
      } else {
        lcd.print(" ");       // empty space
      }
    }
  }

  // Score display on top-right
  lcd.setCursor(12, 0);
  lcd.print("S:");
  lcd.print(score);
  lcd.print("  ");

  // Feedback display on bottom-right
  if (feedbackTop != "") {
    lcd.setCursor(12, 1);
    lcd.print(feedbackTop);
  }
  if (feedbackBottom != "") {
    lcd.setCursor(14, 1);
    lcd.print(feedbackBottom);
  }
}


void generatePattern() {
  randomSeed(analogRead(A0));
  for (int r = 0; r < LCD_ROWS; r++) {
    for (int c = 0; c < LCD_COLS; c++) {
      notes[r][c] = (random(0, 10) < 3) ? 1 : 0;
    }
  }
}

void handleButtons() {
  bool button0 = (digitalRead(buttonPins[0]) == LOW); // top
  bool button1 = (digitalRead(buttonPins[1]) == LOW); // bottom
  int scoreIncrement = 0;

  // -------- TOP ROW --------
  if (button0) {
    if (notes[0][0] == 1 || notes[0][0] == 2) { // Perfect
      scoreIncrement++;
      notes[0][0] = 0;
      feedbackTop = "P";
      feedbackTimeTop = millis();
    } else if (notes[0][1] == 1) {              // Good
      feedbackTop = "G";
      feedbackTimeTop = millis();
    } else {                                    // Miss
      feedbackTop = "M";
      feedbackTimeTop = millis();
    }
  }

  // -------- BOTTOM ROW --------
  if (button1) {
    if (notes[1][0] == 1 || notes[1][0] == 2) { // Perfect
      scoreIncrement++;
      notes[1][0] = 0;
      feedbackBottom = "P";
      feedbackTimeBottom = millis();
    } else if (notes[1][1] == 1) {              // Good
      feedbackBottom = "G";
      feedbackTimeBottom = millis();
    } else {                                    // Miss
      feedbackBottom = "M";
      feedbackTimeBottom = millis();
    }
  }

  // Update global score
  score += scoreIncrement;

  // Refresh display
  if (button0 || button1) displayNotes();
}



