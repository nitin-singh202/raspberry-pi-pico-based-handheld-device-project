#include <Arduino.h>
#include <SPI.h>
#include <Ucglib.h>

// Display configuration
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BUZZER_PIN 5

// Define TFT pins - adjusted according to second file
#define TFT_CS   17
#define TFT_DC   15
#define TFT_RST  16

// Initialize UCGLIB display in landscape mode
Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

// Colors
#define COLOR_BLACK     0, 0, 0
#define COLOR_WHITE     255, 255, 255
#define COLOR_RED       255, 0, 0
#define COLOR_GREEN     0, 255, 0
#define COLOR_BLUE      0, 0, 255
#define COLOR_YELLOW    255, 255, 0
#define COLOR_DARKGREY  64, 64, 64
#define COLOR_NAVY      0, 0, 128
#define COLOR_CYAN      0, 255, 255

// Struct to hold a question, its answer and time limit
struct WordProblem {
  const char* question;
  int answer;       // Answer for the word problem
  int timeLimit;    // Time limit to answer in seconds
};

// Word Problems organized by difficulty
// Easy Word Problems (Real-World Based)
const WordProblem easyQuestions[] = {
  {"You bought 3 items. One costs $5, the second costs $8, and the third costs $3. How much did you spend in total?", 16, 15},
  {"You have 10 apples. You give 3 apples to your friend. How many apples do you have left?", 7, 12},
  {"You are baking a cake and need 4 eggs. You have a carton with 12 eggs. How many eggs will you have left after using 4?", 8, 12},
  {"You are walking at a speed of 2 miles per hour. How far will you walk in 5 hours?", 10, 12},
  {"You have 15 candies and share them equally with 2 friends. How many candies does each person get?", 5, 12},
  {"Your book has 24 pages and you read 6 pages every day. How many days will it take to finish the book?", 4, 15}
};

// Medium Word Problems (Real-World Based)
const WordProblem mediumQuestions[] = {
  {"You want to buy 4 notebooks. Each notebook costs $7. How much money do you need?", 28, 10},
  {"A car travels 60 miles per hour. How far will it travel in 3 hours?", 180, 12},
  {"You need 2 cups of sugar for a recipe. You only have a 1/2 cup measuring spoon. How many times do you need to fill the spoon?", 4, 15},
  {"You and your friends buy 2 pizzas, each with 8 slices. If there are 4 of you, how many slices does each person get?", 4, 15},
  {"A train travels at 80 km/h. How far will it travel in 4 hours?", 320, 20},
  {"If 3 workers can build a wall in 6 hours, how many hours would it take for 6 workers to build the same wall?", 3, 20}
};

// Hard Word Problems (Real-World Based)
const WordProblem hardQuestions[] = {
  {"A factory produces 150 toys every hour. How many toys will the factory produce in 12 hours?", 1800, 15},
  {"You have 180 candies and want to share them equally among 9 children. How many candies will each child get?", 20, 15},
  {"You have $50. You buy 3 items costing $12, $15, and $18. How much money will you have left?", 5, 18},
  {"You are driving at 70 miles per hour. How long will it take you to travel 210 miles?", 3, 18},
  {"A baker has 400 grams of flour. The recipe needs 50 grams per batch. How many batches can the baker make?", 8, 20},
  {"If 5 people can paint a house in 8 days, how many days would it take for 10 people to paint the same house?", 4, 25}
};

// Initial question level
int currentLevel = 0;  // 0 - easy, 1 - medium, 2 - hard
int score = 0;  // Added score tracking from second code

// Store the current question
WordProblem currentQuestion;

// Game state variables
unsigned long startTime;
unsigned long answerTime;
bool answeredCorrectly = false;
bool gameRunning = true;
bool awaitingInput = false;  // Added from second code

// Keypad setup - adjusted to match second file
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
const int rowPins[ROWS] = {13, 12, 11, 10}; // Connect to the row pinouts of the keypad
const int colPins[COLS] = {9, 8, 7, 6}; // Connect to the column pinouts of the keypad

String userInputStr = "";
const int MAX_INPUT_LENGTH = 6;

// Function to clear screen - replaces fillScreen()
void clearScreen() {
  ucg.setColor(COLOR_BLACK);
  ucg.drawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

// Function to draw centered text similar to second file
void drawCenteredText(String text, int y, int size = 2, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255) {
  ucg.setColor(r, g, b);
  ucg.setFont(ucg_font_helvB12_tr);  // Reduced font size
  int x = (SCREEN_WIDTH - (text.length() * 10)) / 2;  // Adjusted calculation for smaller font
  ucg.setPrintPos(x, y);
  ucg.print(text);
}

// Function to draw text with word wrapping
void drawWrappedText(int x, int y, int maxWidth, const char* text, int font_height) {
  int cursor_x = x;
  int cursor_y = y;
  
  // Select font
  ucg.setFont(ucg_font_ncenR12_hr);
  
  // Get a local copy of the text we can manipulate
  String remainingText = String(text);
  String currentWord = "";
  String currentLine = "";
  
  while (remainingText.length() > 0) {
    // Find the next space or end of string
    int spacePos = remainingText.indexOf(' ');
    
    if (spacePos == -1) {
      // No more spaces, take the rest of the text
      currentWord = remainingText;
      remainingText = "";
    } else {
      // Extract the next word
      currentWord = remainingText.substring(0, spacePos + 1);
      remainingText = remainingText.substring(spacePos + 1);
    }
    
    // Check if adding the next word would exceed the line width
    int wordWidth = currentWord.length() * 8; // Approximate width calculation
    int lineWidth = currentLine.length() * 8;
    
    if (lineWidth + wordWidth > maxWidth) {
      // Line would be too long, print current line and start a new one
      ucg.setPrintPos(cursor_x, cursor_y);
      ucg.print(currentLine);
      
      currentLine = currentWord;
      cursor_y += font_height;
    } else {
      // Add word to current line
      currentLine += currentWord;
    }
  }
  
  // Print any remaining text
  if (currentLine.length() > 0) {
    ucg.setPrintPos(cursor_x, cursor_y);
    ucg.print(currentLine);
  }
}

// Display score (added from second file)
void drawScore() {
  ucg.setColor(COLOR_WHITE);
  ucg.setPrintPos(10, 20);
  ucg.setFont(ucg_font_helvB08_tr);  // Reduced font size
  ucg.print("Score: " + String(score));
}

void displayQuestion() {
  // Clear screen
  clearScreen();
  
  // Display score
  drawScore();
  
  // Display title
  ucg.setColor(COLOR_WHITE);
  ucg.setFont(ucg_font_ncenR12_hr);
  ucg.setPrintPos(10, 40);
  ucg.print("Solve the problem:");
  
  // Removed difficulty level display
  
  // Display question
  ucg.setColor(COLOR_WHITE);
  ucg.setFont(ucg_font_ncenR12_hr);
  drawWrappedText(10, 60, SCREEN_WIDTH - 20, currentQuestion.question, 20);
  
  // Display time bar outline
  ucg.setColor(COLOR_WHITE);
  ucg.drawFrame(10, 130, SCREEN_WIDTH - 20, 20);
  
  // Display input area - updated to match second code's style
  ucg.setColor(COLOR_YELLOW);
  ucg.drawBox(60, 160, 200, 40);
  ucg.setColor(COLOR_BLACK);
  ucg.drawFrame(60, 160, 200, 40);
  
  ucg.setColor(COLOR_WHITE);
  ucg.setFont(ucg_font_ncenR14_hr);  // Reduced font size
  ucg.setPrintPos(70, 185);
  ucg.print("Answer: " + userInputStr);
}

void updateTimeBar() {
  int timeLimit = currentQuestion.timeLimit * 1000;
  int elapsed = millis() - startTime;
  int remaining = max(0, timeLimit - elapsed);
  int barWidth = map(remaining, 0, timeLimit, 0, SCREEN_WIDTH - 22);
  
  // Clear and redraw time bar
  ucg.setColor(COLOR_BLACK);
  ucg.drawBox(11, 131, SCREEN_WIDTH - 22, 18);
  
  // Color changes based on remaining time
  if (remaining > timeLimit * 0.6) {
    ucg.setColor(COLOR_GREEN);
  } else if (remaining > timeLimit * 0.3) {
    ucg.setColor(COLOR_YELLOW);
  } else {
    ucg.setColor(COLOR_RED);
  }
  
  ucg.drawBox(11, 131, barWidth, 18);
}

void updateUserInput() {
  // Clear input field - updated to match second code
  ucg.setColor(COLOR_YELLOW);
  ucg.drawBox(60, 160, 200, 40);
  ucg.setColor(COLOR_BLACK);
  ucg.drawFrame(60, 160, 200, 40);
  
  // Display current input
  ucg.setColor(COLOR_BLACK);
  ucg.setFont(ucg_font_ncenR14_hr);  // Reduced font size
  ucg.setPrintPos(70, 185);
  ucg.print("Answer: " + userInputStr);
}

// Flash screen effect from second code - replaced fillScreen with drawBox
void flashScreen() {
  for (int i = 0; i < 2; i++) {
    clearScreen();
    delay(100);
    ucg.setColor(COLOR_WHITE);
    ucg.drawBox(0, 0, 320, 240);
    delay(100);
  }
  clearScreen();
}

void showOverlay(const char* message, const char* button) {
  int overlayW = SCREEN_WIDTH / 1.5;
  int overlayH = SCREEN_HEIGHT / 2;
  int x = (SCREEN_WIDTH - overlayW) / 2;
  int y = (SCREEN_HEIGHT - overlayH) / 2;

  // Draw overlay background
  ucg.setColor(COLOR_DARKGREY);
  ucg.drawBox(x, y, overlayW, overlayH);
  ucg.setColor(COLOR_WHITE);
  ucg.drawFrame(x, y, overlayW, overlayH);
  
  // Draw message - moved upward slightly
  ucg.setColor(COLOR_WHITE);
  ucg.setFont(ucg_font_ncenR14_hr);  // Reduced font size
  
  // Center the message in the overlay
  int textWidth = strlen(message) * 10; // Approximate width calculation
  int textX = x + (overlayW - textWidth) / 2;
  ucg.setPrintPos(textX, y + 40);  // Moved up from y + 50
  ucg.print(message);
  
  // Show the correct answer if incorrect - moved upward too
  if (strstr(message, "Incorrect") != NULL) {
    ucg.setFont(ucg_font_ncenR10_hr);
    ucg.setPrintPos(x + 20, y + 100);  // Moved up from y + 80
    ucg.print("Answer: ");
    ucg.print(currentQuestion.answer);
  }
  
  // Display current score - moved upward
  ucg.setFont(ucg_font_ncenR10_hr);
  ucg.setPrintPos(x + 20, y + 90);  // Moved up from y + 110
  ucg.print("Score: ");
  ucg.print(score);
  
  // Button - moved upward by 20px
  int buttonW = strlen(button) * 14 + 20;
  int buttonX = x + (overlayW - buttonW) / 2;
  
  ucg.setColor(COLOR_BLUE);
  ucg.drawBox(buttonX, y + overlayH - 75, buttonW, 30);  // Moved up from y + overlayH - 55
  ucg.setColor(COLOR_WHITE);
  ucg.drawFrame(buttonX, y + overlayH - 75, buttonW, 30);  // Moved up from y + overlayH - 55
  
  // Center button text - moved upward
  ucg.setFont(ucg_font_ncenR10_hr);
  ucg.setPrintPos(buttonX + 10, y + overlayH - 50);  // Moved up from y + overlayH - 30
  ucg.print(button);
}

void checkAnswer(int userAnswer) {
  // Stop the timer
  answerTime = millis() - startTime;
  
  // Check if the answer is correct
  if (userAnswer == currentQuestion.answer) {
    answeredCorrectly = true;
    score++; // Increment score when correct
    tone(BUZZER_PIN, 1000, 200); // Correct answer feedback
    showOverlay("Correct!", "Next");
    
    // Increase difficulty level if player answers quickly
    if (answerTime <= currentQuestion.timeLimit * 1000 / 2) {
      if (currentLevel < 2) {
        currentLevel++;
      }
    }
  } else {
    answeredCorrectly = false;
    // REMOVED: score = 0; - No longer reset score on wrong answers
    tone(BUZZER_PIN, 300, 200); // Incorrect answer feedback
    showOverlay("Incorrect", "Retry");
    
    // Decrease difficulty if needed
    if (currentLevel > 0) {
      currentLevel--;
    }
  }
}

// Added countdown function from second code
void startCountdown() {
  clearScreen();
  drawScore();
  for (int i = 3; i > 0; i--) {
    drawCenteredText("Get Ready: " + String(i), 120, 3, 255, 255, 0);
    delay(1000);
    clearScreen();
    drawScore();
  }
}

void getNextQuestion() {
  // Clear any previous input
  userInputStr = "";
  
  // Get the next question based on difficulty level
  int questionIndex;
  
  if (currentLevel == 0) {
    questionIndex = random(0, sizeof(easyQuestions) / sizeof(easyQuestions[0]));
    currentQuestion = easyQuestions[questionIndex];
  } else if (currentLevel == 1) {
    questionIndex = random(0, sizeof(mediumQuestions) / sizeof(mediumQuestions[0]));
    currentQuestion = mediumQuestions[questionIndex];
  } else {
    questionIndex = random(0, sizeof(hardQuestions) / sizeof(hardQuestions[0]));
    currentQuestion = hardQuestions[questionIndex];
  }
  
  // Added countdown before showing question
  startCountdown();
  
  displayQuestion();
  updateUserInput();
  startTime = millis();
  answeredCorrectly = false;
  awaitingInput = true;
}

// Implementing keypad reading function from second code
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
        return keys[row][col];
      }
    }
    digitalWrite(rowPins[row], LOW);
    pinMode(rowPins[row], INPUT);
  }
  return '\0';
}

void handleKeypadInput(char key) {
  // Handle number inputs
  if (isdigit(key) && userInputStr.length() < MAX_INPUT_LENGTH) {
    userInputStr += key;
    updateUserInput();
  }
  // Handle delete (using * as backspace)
  else if (key == '*' && userInputStr.length() > 0) {
    userInputStr = userInputStr.substring(0, userInputStr.length() - 1);
    updateUserInput();
  }
  // Handle submit (using # as enter)
  else if (key == '#' && userInputStr.length() > 0) {
    awaitingInput = false;
    int userAnswer = userInputStr.toInt();
    checkAnswer(userAnswer);
  }
  // We've removed the "Handle continue after result" section
}

void showStartScreen() {
  // Clear screen
  ucg.setColor(COLOR_NAVY);
  ucg.drawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  
  // Draw title
  ucg.setColor(COLOR_WHITE);
  ucg.setFont(ucg_font_ncenR18_hr);  // Reduced font size
  ucg.setPrintPos(30, 60);
  ucg.print("MATH PROBLEMS");
  
  ucg.setFont(ucg_font_ncenR14_hr);  // Reduced font size
  ucg.setPrintPos(60, 100);
  ucg.print("Word Problem Game");
  
  // Instructions
  ucg.setFont(ucg_font_ncenR10_hr);  // Reduced font size
  ucg.setPrintPos(20, 220);
  ucg.print("Use keypad to answer. # to submit.");
  
  // Added a message to indicate auto-start
  ucg.setFont(ucg_font_ncenR12_hr);
  ucg.setPrintPos(60, 170);
  ucg.print("Game will start soon...");
}

void setup() {
  // Initialize serial for debugging
  Serial.begin(9600);
  
  // Initialize the display for landscape mode
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.clearScreen();  // This is likely implemented in the library
  // Rotate display to landscape 
  ucg.setRotate90();
  
  // Initialize buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Set random seed
  randomSeed(analogRead(A0));
  
  // Reset score
  score = 0;
  
  // Show start screen
  showStartScreen();
  
  // Short delay before starting the game
  delay(2000);  // Reduced from 3000ms to 2000ms
  
  // Flash screen effect from second code
  flashScreen();
  
  // Start the game
  getNextQuestion();
}

void loop() {
  // Get keypad input regardless of game state
  char key = getKeypadKey();
  
  // Update time bar if awaiting input
  if (awaitingInput) {
    updateTimeBar();
  
    // Check if time is up
    answerTime = millis() - startTime;
    if (answerTime >= currentQuestion.timeLimit * 1000 && !answeredCorrectly) {
      awaitingInput = false;
      tone(BUZZER_PIN, 200, 500); // Time's up feedback
      showOverlay("Time's Up!", "Next");
      
      // Score is no longer reset on time's up either
    }
  
    // Process the player's input from the keypad - only for answering
    if (key != '\0') {
      handleKeypadInput(key);
    }
  } 
  // Handle key presses when awaiting "Next" or "Retry" action
  else if (key != '\0') {
    // Any key press will advance to the next question
    getNextQuestion();
  }
  
  // Small delay to prevent excessive CPU usage
  delay(50);
}