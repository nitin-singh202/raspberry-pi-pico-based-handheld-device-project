#include <Ucglib.h>

// TFT Pins
#define TFT_CS   17
#define TFT_DC   15
#define TFT_RST  16
#define BUZZER_PIN 5

Ucglib_ILI9341_18x240x320_HWSPI tft(TFT_DC, TFT_CS, TFT_RST);

// Keypad pins
const int rowPins[4] = {13, 12, 11, 10};
const int colPins[4] = {9, 8, 7, 6};
char keypadKeys[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Game state
String currentNumber = "";
String userInput = "";
int level = 1;
int score = 0;
bool awaitingInput = false;

void drawCenteredText(String text, int y, int size = 2, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255) {
  tft.setColor(r, g, b);
  tft.setFont(ucg_font_helvB14_tr);
  int x = (320 - (text.length() * 12)) / 2;
  tft.setPrintPos(x, y);
  tft.print(text);
}

void drawScore() {
  tft.setColor(255, 255, 255);
  tft.setPrintPos(10, 20);
  tft.setFont(ucg_font_helvB10_tr);
  tft.print("Score: " + String(score));
}

void flashScreen() {
  for (int i = 0; i < 2; i++) {
    tft.clearScreen();
    delay(100);
    tft.setColor(255, 255, 255);
    tft.drawBox(0, 0, 320, 240);
    delay(100);
  }
  tft.clearScreen();
}

String generateNumber(int digits) {
  String num = "";
  for (int i = 0; i < digits; i++) {
    num += String(random(10));
  }
  return num;
}

void showNumber(String num) {
  tft.clearScreen();
  drawScore();
  drawCenteredText(num, 120, 4, 0, 255, 255);
  delay(2000);
  flashScreen();
}

void startCountdown() {
  tft.clearScreen();
  drawScore();
  for (int i = 5; i > 0; i--) {
    drawCenteredText("Get Ready: " + String(i), 120);
    delay(1000);
    tft.clearScreen();
    drawScore();
  }
}

void askForInput() {
  tft.clearScreen();
  drawScore();
  drawCenteredText("Enter the number:", 50);
  userInput = "";
  awaitingInput = true;
}

void updateInputDisplay() {
  tft.setColor(255, 255, 0);
  tft.drawBox(60, 100, 200, 40);
  tft.setColor(0, 0, 0);
  tft.drawFrame(60, 100, 200, 40);
  drawCenteredText(userInput, 125);
}

void showResult(bool correct) {
  tft.clearScreen();
  if (correct) {
    drawCenteredText("Correct!", 60, 3, 0, 255, 0);
    score++;
    level++;
  } else {
    drawCenteredText("Wrong!", 60, 3, 255, 0, 0);
    score = 0;
    level = 1;
  }

  drawCenteredText("Score: " + String(score), 140);
  if (BUZZER_PIN) {
    tone(BUZZER_PIN, correct ? 1000 : 200, 300);
  }
  delay(3000);
}

char getKeypadKey() {
  for (int row = 0; row < 4; row++) {
    pinMode(rowPins[row], OUTPUT);
    digitalWrite(rowPins[row], HIGH);
    for (int col = 0; col < 4; col++) {
      pinMode(colPins[col], INPUT_PULLDOWN);
      if (digitalRead(colPins[col]) == HIGH) {
        delay(200);
        digitalWrite(rowPins[row], LOW);
        pinMode(rowPins[row], INPUT);
        return keypadKeys[row][col];
      }
    }
    digitalWrite(rowPins[row], LOW);
    pinMode(rowPins[row], INPUT);
  }
  return '\0';
}

void setup() {
  tft.begin(UCG_FONT_MODE_TRANSPARENT);
  tft.setRotate90();
  randomSeed(analogRead(A0));
  pinMode(BUZZER_PIN, OUTPUT);

  tft.clearScreen();
  drawCenteredText("Number Memory Game", 100);
  delay(2000);

  score = 0;
  currentNumber = generateNumber(level + 1);
  showNumber(currentNumber);
  startCountdown();
  askForInput();
}

void loop() {
  if (awaitingInput) {
    char key = getKeypadKey();
    if (key != '\0') {
      if (key == '#') {
        awaitingInput = false;
        bool correct = (userInput == currentNumber);
        showResult(correct);
        currentNumber = generateNumber(level + 1);
        showNumber(currentNumber);
        startCountdown();
        askForInput();
      } else if (key == '*') {
        if (userInput.length() > 0) {
          userInput.remove(userInput.length() - 1);
          updateInputDisplay();
        }
      } else if (key >= '0' && key <= '9') {
        if (userInput.length() < 20) {
          userInput += key;
          updateInputDisplay();
        }
      }
    }
  }
}
