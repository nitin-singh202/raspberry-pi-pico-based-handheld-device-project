#ifndef MATH_SEQUENCE_GAME_H
#define MATH_SEQUENCE_GAME_H

#include <Arduino.h>
#include <SPI.h>
#include <Ucglib.h>

// Display configuration
#define TFT_CS   17
#define TFT_DC   15
#define TFT_RST  16
#define BUZZER_PIN 5
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Create display instance
Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

// RGB Color definitions
#define COLOR_BLACK 0, 0, 0
#define COLOR_WHITE 255, 255, 255
#define COLOR_DARKGREY 100, 100, 100
#define COLOR_YELLOW 255, 255, 0
#define COLOR_GREEN 0, 255, 0
#define COLOR_RED 255, 0, 0
#define COLOR_BLUE 0, 0, 255

// Keypad pins
const int rowPins[4] = {13, 12, 11, 10};
const int colPins[4] = {9, 8, 7, 6};

// Keypad layout
const char KEYPAD[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Game variables
int currentAnswer = 0;
String inputBuffer = "";
int score = 0;
bool waitingForAnswer = false;
unsigned long gameStartTime = 0;
int questionCount = 0;
int maxQuestions = 10;
bool gameOver = false;

// Audio feedback functions
void playCorrectTone() {
  tone(BUZZER_PIN, 1000, 150);
  delay(200);
  tone(BUZZER_PIN, 1200, 150);
  delay(200);
  noTone(BUZZER_PIN);
}

void playWrongTone() {
  tone(BUZZER_PIN, 400, 200);
  delay(200);
  tone(BUZZER_PIN, 300, 200);
  delay(200);
  tone(BUZZER_PIN, 200, 300);
  delay(300);
  noTone(BUZZER_PIN);
}

void playGameOverSound() {
  tone(BUZZER_PIN, 400, 200);
  delay(200);
  tone(BUZZER_PIN, 300, 200);
  delay(200);
  tone(BUZZER_PIN, 200, 300);
  delay(300);
  noTone(BUZZER_PIN);
}

// Keypad input
char getKey() {
  for (int row = 0; row < 4; row++) {
    pinMode(rowPins[row], OUTPUT);
    digitalWrite(rowPins[row], HIGH);

    for (int col = 0; col < 4; col++) {
      pinMode(colPins[col], INPUT_PULLDOWN);
      if (digitalRead(colPins[col]) == HIGH) {
        delay(200);
        digitalWrite(rowPins[row], LOW);
        pinMode(rowPins[row], INPUT);
        return KEYPAD[row][col];
      }
    }

    digitalWrite(rowPins[row], LOW);
    pinMode(rowPins[row], INPUT);
  }
  return '\0';
}

// Draw the score in bottom-right
void drawScore() {
  ucg.setColor(COLOR_BLACK);
  ucg.drawBox(SCREEN_WIDTH - 130, SCREEN_HEIGHT - 40, 120, 30);

  ucg.setColor(COLOR_YELLOW);
  ucg.setFont(ucg_font_ncenR12_tr);
  ucg.setPrintPos(SCREEN_WIDTH - 120, SCREEN_HEIGHT - 20);
  ucg.print("Score: ");
  ucg.print(score);
}

// Show a message box
void showOverlay(const char* message, const char* option) {
  int overlayW = SCREEN_WIDTH / 2;
  int overlayH = SCREEN_HEIGHT / 2;
  int x = (SCREEN_WIDTH - overlayW) / 2;
  int y = (SCREEN_HEIGHT - overlayH) / 2;

  ucg.setColor(COLOR_DARKGREY);
  ucg.drawBox(x, y, overlayW, overlayH);
  
  ucg.setColor(COLOR_WHITE);
  ucg.setFont(ucg_font_ncenR14_hr);
  ucg.setPrintPos(x + 10, y + 30);
  ucg.print(message);

  ucg.setFont(ucg_font_ncenR12_hr);
  ucg.setPrintPos(x + 30, y + 80);
  ucg.setColor(COLOR_YELLOW);
  ucg.print(">> ");
  ucg.print(option);
}

// Show input field
void updateInputDisplay() {
  ucg.setColor(COLOR_BLACK);
  ucg.drawBox(30, 170, 260, 30);

  ucg.setColor(COLOR_BLUE);
  ucg.drawFrame(30, 170, 260, 30);

  ucg.setPrintPos(40, 190);
  ucg.setFont(ucg_font_ncenR14_hr);
  ucg.setColor(COLOR_WHITE);
  ucg.print(inputBuffer);
}

// Generate and display sequence
void generateSequence() {
  ucg.setColor(COLOR_BLACK);
  ucg.drawBox(0, 50, SCREEN_WIDTH, 100);
  
  ucg.setColor(COLOR_YELLOW);
  ucg.drawBox(30, 60, 260, 50);
  ucg.setColor(COLOR_RED);
  ucg.drawFrame(30, 60, 260, 50);
  
  ucg.setFont(ucg_font_ncenR18_hr);
  ucg.setColor(COLOR_BLACK);
  ucg.setPrintPos(40, 95);

  int a = random(2, 10);
  int b = random(2, 5);
  int step = random(0, 3);
  String sequenceText = "";

  if (score > 5) {
    a = random(5, 12);
    b = random(2, 4);
  }

  if (step == 0) {
    currentAnswer = a + b*3;
    sequenceText = String(a) + ", " + String(a + b) + ", " + String(a + b*2) + ", ?";
  } else if (step == 1) {
    a = 20 + random(0, 10);
    currentAnswer = a - b*3;
    sequenceText = String(a) + ", " + String(a - b) + ", " + String(a - b*2) + ", ?";
  } else {
    if (b > 2) b = 2;
    currentAnswer = a * (b*b*b);
    sequenceText = String(a) + ", " + String(a * b) + ", " + String(a * b*b) + ", ?";
  }

  ucg.print(sequenceText);

  // Instructions split into 2 lines
  ucg.setFont(ucg_font_ncenR12_hr);
  ucg.setColor(COLOR_WHITE);
  ucg.setPrintPos(30, 135);
  ucg.print("Enter the missing number");
  ucg.setPrintPos(30, 150);
  ucg.print("(press # to submit):");

  inputBuffer = "";
  waitingForAnswer = true;
  updateInputDisplay();
}

// Game over display
void showGameOverScreen() {
  playGameOverSound();

  int boxW = SCREEN_WIDTH * 2 / 3;
  int boxH = SCREEN_HEIGHT / 2;
  int boxX = (SCREEN_WIDTH - boxW) / 2;
  int boxY = (SCREEN_HEIGHT - boxH) / 2;

  ucg.setColor(COLOR_RED);
  ucg.drawBox(boxX, boxY, boxW, boxH);
  ucg.setColor(COLOR_WHITE);
  ucg.drawFrame(boxX, boxY, boxW, boxH);

  ucg.setColor(COLOR_WHITE);
  ucg.setFont(ucg_font_ncenR14_hr);
  ucg.setPrintPos(boxX + 10, boxY + 25);
  ucg.print("Game Over!");

  ucg.setPrintPos(boxX + 10, boxY + 50);
  ucg.print("Final Score: ");
  ucg.print(score);
  
  ucg.setPrintPos(boxX + 10, boxY + 75);
  ucg.print("Questions: ");
  ucg.print(questionCount);
  ucg.print("/");
  ucg.print(maxQuestions);

  ucg.setPrintPos(boxX + 10, boxY + 100);
  ucg.print("Press # to play again");
}

// Reset game to start
void resetGame() {
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.setRotate90();
  ucg.clearScreen();
  
  ucg.setColor(COLOR_GREEN);
  ucg.drawBox(10, 5, 300, 30);
  ucg.setColor(COLOR_BLACK);
  ucg.setFont(ucg_font_ncenR14_hr);
  ucg.setPrintPos(20, 25);
  ucg.print("Math Sequence Challenge");

  score = 0;
  questionCount = 0;
  gameOver = false;
  waitingForAnswer = false;
  inputBuffer = "";

  drawScore();
  generateSequence();
  gameStartTime = millis();
}

// Handle keypad input
void processInput(char key) {
  if (gameOver) {
    if (key == '#') {
      resetGame();
    }
    return;
  }

  if (!waitingForAnswer) return;

  if (key >= '0' && key <= '9' && inputBuffer.length() < 5) {
    inputBuffer += key;
    updateInputDisplay();
  } 
  else if (key == '*' && inputBuffer.length() > 0) {
    inputBuffer.remove(inputBuffer.length() - 1);
    updateInputDisplay();
  }
  else if (key == '#' && inputBuffer.length() > 0) {
    int userAnswer = inputBuffer.toInt();
    waitingForAnswer = false;
    questionCount++;

    if (userAnswer == currentAnswer) {
      playCorrectTone();
      score++;

      ucg.setColor(COLOR_GREEN);
      ucg.drawBox(60, 120, 200, 30);
      ucg.setColor(COLOR_WHITE);
      ucg.setPrintPos(70, 140);
      ucg.print("Correct! +1 point");

      drawScore();
      delay(1000);
    } else {
      playWrongTone();

      ucg.setColor(COLOR_RED);
      ucg.drawBox(60, 120, 200, 30);
      ucg.setColor(COLOR_WHITE);
      ucg.setPrintPos(70, 140);
      ucg.print("Wrong! Answer: ");
      ucg.print(currentAnswer);

      delay(2000);
    }

    if (questionCount >= maxQuestions) {
      gameOver = true;
      showGameOverScreen();
    } else {
      generateSequence();
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  randomSeed(analogRead(A2));
  resetGame();
}

void loop() {
  char key = getKey();
  if (key != '\0') {
    Serial.print("Key pressed: ");
    Serial.println(key);
    processInput(key);
  }
  delay(100);
}

#endif
