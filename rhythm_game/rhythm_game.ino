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

unsigned long feedbackTime = 0;
String feedbackText = "";

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

  // Handle button presses for hits
  handleButtons();

  // Clear feedback after short delay
  if (feedbackText != "" && millis() - feedbackTime > 500) {
    feedbackText = "";
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
    if (notes[r][1] == 1) {
      notes[r][0] = 2; // entering hit zone
    } else if (notes[r][0] == 2) {
      notes[r][0] = 0; // clear flash
    }

    // Shift all notes left
    for (int c = 1; c < LCD_COLS - 1; c++) {
      notes[r][c] = notes[r][c + 1];
    }

    // Random spawn on far right
    notes[r][LCD_COLS - 1] = (random(0, 10) < 3) ? 1 : 0;
  }
}

void displayNotes() {
  lcd.clear();

  for (int r = 0; r < LCD_ROWS; r++) {
    for (int c = 0; c < LCD_COLS; c++) {
      lcd.setCursor(c, r);

      if (c == 0) {
        if (notes[r][0] == 2)
          lcd.write(byte(2)); // flash
        else
          lcd.write(byte(1)); // always show hit zone
      } else if (notes[r][c] == 1) {
        lcd.write(byte(0)); // moving note
      } else {
        lcd.print(" ");
      }
    }
  }

  // Score display
  lcd.setCursor(12, 0);
  lcd.print("S:");
  lcd.print(score);
  lcd.print("  ");

  // Feedback display
  if (feedbackText != "") {
    lcd.setCursor(13, 1);
    lcd.print(feedbackText);
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
// -------------------- BUTTON HANDLER FOR L2D --------------------
void handleButtons() {
  bool button0 = (digitalRead(buttonPins[0]) == LOW);
  bool button1 = (digitalRead(buttonPins[1]) == LOW);
  int scoreIncrement = 0;
  String newFeedback = "";

  // Top row
  if (button0) {
    if (notes[0][0] == 1 || notes[0][0] == 2) {
      scoreIncrement++;
      notes[0][0] = 0;       // clear note
      newFeedback = "P";
    } else if (notes[0][1] == 1) {
      newFeedback = "G";
    } else {
      newFeedback = "M";
    }
  }

  // Bottom row
  if (button1) {
    if (notes[1][0] == 1 || notes[1][0] == 2) {
      scoreIncrement++;
      notes[1][0] = 0;       // clear note
      if (newFeedback != "P") newFeedback = "P"; // keep best feedback
    } else if (notes[1][1] == 1) {
      if (newFeedback != "P") newFeedback = "G";
    } else {
      if (newFeedback == "") newFeedback = "M";
    }
  }

  // Update global score and feedback if any button pressed
  if (button0 || button1) {
    score += scoreIncrement;
    feedbackText = newFeedback;
    feedbackTime = millis();
    displayNotes();
  }
}

