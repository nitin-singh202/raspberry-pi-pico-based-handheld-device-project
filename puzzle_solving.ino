#include <Arduino.h>
#include <Ucglib.h>
#include <SPI.h>

// TFT Pins (based on the reference code)
#define TFT_CS   17
#define TFT_DC   15
#define TFT_RST  16
#define BUZZER_PIN 5  // Changed to match reference code

// Display dimensions
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Create Ucglib TFT instance  
Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

// Colors for Ucglib - RGB format (matching reference code)
#define BLACK    0, 0, 0
#define WHITE    255, 255, 255
#define RED      255, 0, 0
#define GREEN    0, 255, 0
#define BLUE     0, 0, 255
#define YELLOW   255, 255, 0
#define CYAN     0, 255, 255
#define MAGENTA  255, 0, 255
#define DARKGREY 64, 64, 64

// Input handling
#define JOY_X      A0
#define JOY_Y      A1
#define JOY_BTN    4
unsigned long lastJoystickMove = 0;

// Layout Constants for 2x2 grid (matching reference code style)
const int OPTION_WIDTH = 80;    // Width for each option box
const int OPTION_HEIGHT = 40;   // Height for each option box
const int OPTION_SPACING = 10;  // Space between option boxes
const int OPTIONS_AREA_Y = 140; // Starting Y position for options area

int puzzleScore = 0;
int puzzleType = 0;
bool awaitingInput = true;
char correctCharAnswer;
int correctNumAnswer;
String currentQuestion;
String options[4];  // Changed to 4 for 2x2 grid
int selectedOption = 0;

// Arrays for procedural puzzle generation
String colors[] = {"Red", "Green", "Blue", "Yellow", "Purple", "Orange"};
String shapes[] = {"Square", "Triangle", "Circle", "Diamond", "Star"};
String animals[] = {"Cat", "Dog", "Bird", "Fish", "Lion"};
String objects[] = {"Book", "Chair", "Table", "Lamp", "Clock"};
String letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

void playBuzzer(bool correct) {
  tone(BUZZER_PIN, correct ? 1000 : 300, 300);
  delay(300);
  noTone(BUZZER_PIN);
}

void showOverlay(const char* message, const char* button) {
  ucg.setColor(DARKGREY);
  ucg.drawBox(60, 60, 200, 120);
  ucg.setColor(WHITE);
  ucg.setFont(ucg_font_helvR14_tr);
  ucg.setPrintPos(80, 90);
  ucg.print(message);
  ucg.setPrintPos(90, 140);
  ucg.setColor(YELLOW);
  ucg.print(">> ");
  ucg.print(button);
}

void generateNumberPuzzle() {
  int operation = random(4); // 0: add, 1: subtract, 2: multiply, 3: divide
  int a, b;
  
  switch (operation) {
    case 0: // Addition
      a = random(1, 50);
      b = random(1, 50);
      correctNumAnswer = a + b;
      currentQuestion = String(a) + " + " + String(b) + " = ?";
      break;
      
    case 1: // Subtraction
      a = random(30, 100);
      b = random(1, a); // Ensure b < a for positive result
      correctNumAnswer = a - b;
      currentQuestion = String(a) + " - " + String(b) + " = ?";
      break;
      
    case 2: // Multiplication
      a = random(2, 10);
      b = random(2, 12);
      correctNumAnswer = a * b;
      // Using x instead of × for better display
      currentQuestion = String(a) + " x " + String(b) + " = ?";
      break;
      
    case 3: // Division (with whole number result)
      b = random(2, 9);
      a = b * random(1, 10); // Ensure divisible
      correctNumAnswer = a / b;
      currentQuestion = String(a) + " / " + String(b) + " = ?";
      break;
  }
  
  // Create options for 2x2 grid
  int correctOption = random(0, 4);
  options[correctOption] = String(correctNumAnswer);
  
  // Create wrong options
  for (int i = 0; i < 4; i++) {
    if (i != correctOption) {
      int wrong;
      do {
        wrong = correctNumAnswer + random(-10, 11);
      } while (wrong == correctNumAnswer || wrong < 0);
      options[i] = String(wrong);
    }
  }
  
  selectedOption = 0;
}

void generateWordPuzzle() {
  int wordType = random(4); // 0: animals, 1: objects, 2: colors, 3: shapes
  String word;
  int letterPos;
  
  switch (wordType) {
    case 0:
      word = animals[random(0, 5)];
      break;
    case 1:
      word = objects[random(0, 5)];
      break;
    case 2:
      word = colors[random(0, 6)];
      break;
    case 3:
      word = shapes[random(0, 5)];
      break;
  }
  
  word.toUpperCase(); // Work with uppercase for simplicity
  letterPos = random(0, word.length());
  correctCharAnswer = word.charAt(letterPos);
  
  // Create the masked word
  String maskedWord = "";
  for (int i = 0; i < word.length(); i++) {
    if (i == letterPos) {
      maskedWord += "_";
    } else {
      maskedWord += word.charAt(i);
    }
  }
  
  currentQuestion = "Fill in: " + maskedWord;
  
  // Generate options (one correct, three wrong)
  options[0] = String(correctCharAnswer);
  
  char wrongOptions[3];
  for (int i = 0; i < 3; i++) {
    char wrong;
    do {
      wrong = letters.charAt(random(0, 26));
    } while (wrong == correctCharAnswer || 
             (i > 0 && wrong == wrongOptions[0]) || 
             (i > 1 && wrong == wrongOptions[1]));
    wrongOptions[i] = wrong;
    options[i + 1] = String(wrong);
  }
  
  // Shuffle options
  int correctPos = random(0, 4);
  String temp = options[0];
  options[0] = options[correctPos];
  options[correctPos] = temp;
  
  selectedOption = 0;
}

void generateSequencePuzzle() {
  int sequenceType = random(3); // 0: arithmetic, 1: fibonacci-like, 2: alternating
  String sequence = "";
  int nextNumber;
  
  switch (sequenceType) {
    case 0: { // Arithmetic sequence
      int start = random(1, 10);
      int step = random(1, 5);
      
      for (int i = 0; i < 4; i++) {
        sequence += String(start + i * step);
        if (i < 3) sequence += ", ";  // Add comma between numbers
      }
      
      nextNumber = start + 4 * step;
      break;
    }
    
    case 1: { // Fibonacci-like sequence
      int a = random(1, 5);
      int b = random(1, 10);
      sequence = String(a) + ", " + String(b) + ", ";
      
      int c, d;
      c = a + b;
      sequence += String(c) + ", ";
      
      d = b + c;
      sequence += String(d);
      
      nextNumber = c + d;
      break;
    }
    
    case 2: { // Alternating sequence
      int a = random(1, 10);
      int b = random(1, 10);
      
      while (b == a) b = random(1, 10); // Ensure different
      
      sequence = String(a) + ", " + String(b) + ", " + String(a) + ", " + String(b);
      nextNumber = a;
      break;
    }
  }
  
  correctNumAnswer = nextNumber;
  currentQuestion = sequence + ", ?";
  
  // Generate options
  options[0] = String(nextNumber);
  
  int wrongOptions[3];
  for (int i = 0; i < 3; i++) {
    int wrong;
    do {
      wrong = nextNumber + random(-5, 6);
    } while (wrong == nextNumber || wrong <= 0 || 
             (i > 0 && wrong == wrongOptions[0]) || 
             (i > 1 && wrong == wrongOptions[1]));
    wrongOptions[i] = wrong;
    options[i + 1] = String(wrong);
  }
  
  // Shuffle options
  int correctPos = random(0, 4);
  String temp = options[0];
  options[0] = options[correctPos];
  options[correctPos] = temp;
  
  selectedOption = 0;
}

void drawPuzzleScreen() {
  ucg.setColor(BLACK);
  ucg.drawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  
  // Draw only score (removed level)
  ucg.setColor(WHITE);
  ucg.setFont(ucg_font_helvR14_tr);
  ucg.setPrintPos(10, 20);
  ucg.print("Score: ");
  ucg.print(puzzleScore);
  
  // Draw question
  ucg.setColor(WHITE);
  ucg.setFont(ucg_font_helvR14_tr);
  ucg.setPrintPos(20, 60);
  ucg.print(currentQuestion);
  
  // Draw instructions
  ucg.setFont(ucg_font_helvR10_tr);
  ucg.setColor(WHITE);
  ucg.setPrintPos(20, OPTIONS_AREA_Y - 20);
  ucg.print("Use joystick to select, press to confirm");
  
  // Draw options in 2x2 grid
  int startX = (SCREEN_WIDTH - (2 * OPTION_WIDTH + OPTION_SPACING)) / 2;
  
  for (int i = 0; i < 4; i++) {
    int row = i / 2;
    int col = i % 2;
    int x = startX + col * (OPTION_WIDTH + OPTION_SPACING);
    int y = OPTIONS_AREA_Y + row * (OPTION_HEIGHT + OPTION_SPACING);
    
    // Draw option box
    if (i == selectedOption) {
      ucg.setColor(YELLOW);
      ucg.drawFrame(x, y, OPTION_WIDTH, OPTION_HEIGHT);
      ucg.drawFrame(x+1, y+1, OPTION_WIDTH-2, OPTION_HEIGHT-2); // Double frame for better visibility
    } else {
      ucg.setColor(WHITE);
      ucg.drawFrame(x, y, OPTION_WIDTH, OPTION_HEIGHT);
    }
    
    // Draw the option centered in box
    ucg.setFont(ucg_font_helvR12_tr);
    int textWidth = options[i].length() * 6;
    int textX = x + (OPTION_WIDTH - textWidth) / 2;
    int textY = y + (OPTION_HEIGHT + 8) / 2;
    
    ucg.setColor(WHITE);
    ucg.setPrintPos(textX, textY);
    ucg.print(options[i]);
  }
}

void generatePuzzle() {
  puzzleType = random(3); // 0-2, simplified from 6 to 3 puzzle types
  
  switch (puzzleType) {
    case 0:
      generateNumberPuzzle();
      break;
    case 1:
      generateWordPuzzle();
      break;
    case 2:
      generateSequencePuzzle();
      break;
  }
  
  drawPuzzleScreen();
  awaitingInput = true;
}

void handleJoystickInput() {
  if (!awaitingInput) {
    if (digitalRead(JOY_BTN) == LOW) {
      delay(500);
      generatePuzzle();
    }
    return;
  }
  
  int xVal = analogRead(JOY_X);
  int yVal = analogRead(JOY_Y);
  
  if (millis() - lastJoystickMove > 300) {
    // Left
    if (xVal < 300 && selectedOption % 2 != 0) {
      selectedOption--;
      lastJoystickMove = millis();
      drawPuzzleScreen();
    }
    // Right
    else if (xVal > 700 && selectedOption % 2 == 0 && selectedOption < 3) {
      selectedOption++;
      lastJoystickMove = millis();
      drawPuzzleScreen();
    }
    // Up
    else if (yVal < 300 && selectedOption >= 2) {
      selectedOption -= 2;
      lastJoystickMove = millis();
      drawPuzzleScreen();
    }
    // Down
    else if (yVal > 700 && selectedOption < 2) {
      selectedOption += 2;
      lastJoystickMove = millis();
      drawPuzzleScreen();
    }
  }
  
  if (digitalRead(JOY_BTN) == LOW) {
    // Check if selected option is correct
    bool isCorrect = false;
    
    if (puzzleType == 0 || puzzleType == 2) {
      // Number or sequence puzzle
      if (options[selectedOption].toInt() == correctNumAnswer) {
        isCorrect = true;
      }
    } else if (puzzleType == 1) {
      // Word puzzle
      if (options[selectedOption].charAt(0) == correctCharAnswer) {
        isCorrect = true;
      }
    }
    
    if (isCorrect) {
      playBuzzer(true);
      puzzleScore++;
      showOverlay("Correct!", "Next");
    } else {
      playBuzzer(false);
      showOverlay("Wrong!", "Retry");
    }
    
    delay(1500);
    awaitingInput = false;
  }
}

void startPuzzleGame() {
  ucg.setColor(BLACK);
  ucg.drawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  ucg.setFont(ucg_font_helvR18_tr);
  ucg.setColor(WHITE);
  ucg.setPrintPos(40, 100);
  ucg.print("Puzzle Solving");
  ucg.setFont(ucg_font_helvR14_tr);
  ucg.setPrintPos(60, 140);
  ucg.print("Various mind puzzles");
  delay(2000);
  
  puzzleScore = 0;
  generatePuzzle();
}

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(JOY_BTN, INPUT_PULLUP);
  
  // Initialize the LCD
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.setRotate90(); // Landscape mode
  
  randomSeed(micros()); // Use micros() for better randomness
  
  startPuzzleGame();
}

void loop() {
  handleJoystickInput();
}