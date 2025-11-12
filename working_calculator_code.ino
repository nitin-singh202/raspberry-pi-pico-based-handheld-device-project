#include <Ucglib.h>

// TFT Pins
#define TFT_CS   17
#define TFT_DC   15
#define TFT_RST  16

Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

// Keypad pins
const int rowPins[4] = {13, 12, 11, 10};
const int colPins[4] = {9, 8, 7, 6};

// Keypad key values
const char KEYPAD[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Labels shown on TFT for each key
const char* DISPLAY_LABELS[4][4] = {
  {"1", "2", "3", "+"},
  {"4", "5", "6", "-"},
  {"7", "8", "9", "*"},
  {"DEL", "0", "=", "/"}
};

String expression = "";

// Update drawScreen() to fit better in landscape mode too
void drawScreen() {
  ucg.setColor(255, 255, 55); // yellow bg
  ucg.drawBox(10, 20, 300, 50);  // Wider box for 320px screen

  ucg.setColor(230, 30, 39); // border
  ucg.drawFrame(10, 20, 300, 50);

  String displayText = expression;
  if (expression.length() > 18)
    displayText = expression.substring(expression.length() - 18);

  ucg.setColor(255, 0, 0); // red text
  ucg.setPrintPos(15, 55);
  ucg.setFont(ucg_font_helvB14_tr);
  ucg.print(displayText);
}


// Draw a button with label
void drawButton(int x, int y, int w, int h, const char *label) {
  ucg.setColor(220, 220, 22); // button fill
  ucg.drawBox(x, y, w, h);

  ucg.setColor(230, 30, 39);  // border
  ucg.drawFrame(x, y, w, h);

  ucg.setColor(0, 120, 110);  // text
  ucg.setPrintPos(x + (w / 2) - 10, y + (h / 2) + 5);
  ucg.setFont(ucg_font_helvB12_tr);
  ucg.print(label);
}

// Draw all keypad buttons (Updated for Landscape layout)
void drawKeypad() {
  int buttonWidth = 65;   // 4 buttons x 65 + 3x5 = 320px
  int buttonHeight = 30;  // 4 buttons x 30 + 3x5 + display = fits well in 240px
  int padding = 5;
  int startX = 5;
  int startY = 100;

  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      int x = startX + col * (buttonWidth + padding);
      int y = startY + row * (buttonHeight + padding);
      drawButton(x, y, buttonWidth, buttonHeight, DISPLAY_LABELS[row][col]);
    }
  }
}


// Detect keypress from keypad
char getKey() {
  for (int row = 0; row < 4; row++) {
    pinMode(rowPins[row], OUTPUT);
    digitalWrite(rowPins[row], HIGH);

    for (int col = 0; col < 4; col++) {
      pinMode(colPins[col], INPUT_PULLDOWN);
      if (digitalRead(colPins[col]) == HIGH) {
        delay(200); // debounce
        return KEYPAD[row][col];
      }
    }

    digitalWrite(rowPins[row], LOW);
    pinMode(rowPins[row], INPUT);
  }
  return '\0';
}

// Handle keypress and update expression
void handleKeypress(char key) {
  if (key == '#') {
    expression = String(evalExpression(expression.c_str()));
  } else if (key == '*') {
    if (expression.length() > 0)
      expression.remove(expression.length() - 1);
  } else if (key == 'A') {
    expression += '+';
  } else if (key == 'B') {
    expression += '-';
  } else if (key == 'C') {
    expression += '*';
  } else if (key == 'D') {
    expression += '/';
  } else {
    expression += key;
  }
  drawScreen();
}

// Evaluate simple arithmetic expression
int evalExpression(const char *expr) {
  int result = 0;
  char op = '+';
  int num = 0;
  while (*expr) {
    if (isdigit(*expr)) {
      num = num * 10 + (*expr - '0');
    } else {
      if (op == '+') result += num;
      else if (op == '-') result -= num;
      else if (op == '*') result *= num;
      else if (op == '/') result /= num;
      num = 0;
      op = *expr;
    }
    expr++;
  }
  if (op == '+') result += num;
  else if (op == '-') result -= num;
  else if (op == '*') result *= num;
  else if (op == '/') result /= num;
  return result;
}

// Setup TFT and UI
void setup() {
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.setRotate90(); // ROTATE to LANDSCAPE
  ucg.clearScreen();
  drawScreen();
  drawKeypad();
}

// Main loop
void loop() {
  char key = getKey();
  if (key != '\0') {
    handleKeypress(key);
  }
  delay(100);
}
