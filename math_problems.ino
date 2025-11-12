#include <Ucglib.h>

// Define constants
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BUZZER_PIN 9

#define TFT_CS   17
#define TFT_DC   15
#define TFT_RST  16

// Joystick pins
#define JOYSTICK_X_PIN A0
#define JOYSTICK_Y_PIN A1
#define JOYSTICK_BTN_PIN 5  // Digital pin for joystick button

// Button pins
#define BTN_UP_PIN 2
#define BTN_DOWN_PIN 3
#define BTN_LEFT_PIN 4
#define BTN_RIGHT_PIN 14   // Changed from 18 to 14 as requested
#define BTN_ENTER_PIN 21
#define BTN_BACK_PIN 20

Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

const int rowPins[4] = {13, 12, 11, 10};
const int colPins[4] = {9, 8, 7, 6};

// Math operation options
const char* options[] = {"Addition", "Subtraction", "Multiplication", "Division"};
int selectedOption = 0;

// Difficulty level and variables for math questions
int difficulty = 1;
int operand1, operand2, correctAnswer;
String userInput = "";
unsigned long questionStartTime;
bool awaitingAnswer = false;

// Joystick variables
int joystickXValue = 512;
int joystickYValue = 512;
bool joystickButtonState = false;
bool lastJoystickButtonState = false;
unsigned long lastJoystickMoveTime = 0;
const int joystickDeadzone = 100;  // Deadzone for joystick
const int joystickDebounceTime = 300;  // Time between joystick movements

// Button states
bool btnUpState = false;
bool btnDownState = false;
bool btnLeftState = false;
bool btnRightState = false;
bool btnEnterState = false;
bool btnBackState = false;

bool lastBtnUpState = false;
bool lastBtnDownState = false;
bool lastBtnLeftState = false;
bool lastBtnRightState = false;
bool lastBtnEnterState = false;
bool lastBtnBackState = false;

unsigned long lastButtonPressTime = 0;
const int buttonDebounceTime = 200;  // Time between button presses

// Draw text with custom size
void drawText(const char* text, int x, int y, uint8_t r, uint8_t g, uint8_t b, int size) {
  ucg.setColor(r, g, b);
  
  // Select an appropriate font based on size
  if (size == 1) {
    ucg.setFont(ucg_font_ncenR12_tr);
  } else if (size == 2) {
    ucg.setFont(ucg_font_ncenR14_tr);
  } else {
    ucg.setFont(ucg_font_ncenR18_tr);  // Largest size
  }
  
  ucg.setPrintPos(x, y);
  ucg.print(text);
}

// Draw menu screen
void drawMenu() {
  ucg.clearScreen();
  
  // Draw title
  drawText("Basic Mathematics", (SCREEN_WIDTH - 200) / 2, 40, 255, 255, 255, 3);

  // Draw menu options
  for (int i = 0; i < 4; i++) {
    int y = 80 + i * 30;
    if (i == selectedOption) {
      drawText(">>", 20, y, 255, 255, 0, 2);  // Yellow for selected option
      drawText(options[i], 50, y, 255, 255, 0, 2);
    } else {
      drawText(options[i], 50, y, 255, 255, 255, 2);
    }
  }
  
  // Draw button controls guide
  drawText("UP/DOWN: Select", 20, 200, 128, 128, 255, 1);
  drawText("ENTER: Start", 180, 200, 128, 255, 128, 1);
  drawText("BACK: Exit", 180, 220, 255, 128, 128, 1);
}

// Generate a math question
void generateQuestion() {
  // Generate operands based on difficulty
  operand1 = random(1, 10 * difficulty);
  operand2 = random(1, 10 * difficulty);
  
  // Calculate correct answer based on selected operation
  switch (selectedOption) {
    case 0: correctAnswer = operand1 + operand2; break;
    case 1: correctAnswer = operand1 - operand2; break;
    case 2: correctAnswer = operand1 * operand2; break;
    case 3: 
      operand2 = random(1, 10 * difficulty); 
      correctAnswer = operand1; 
      operand1 = operand1 * operand2; 
      break;
  }

  userInput = "";
  awaitingAnswer = true;
  questionStartTime = millis();

  // Display question
  ucg.clearScreen();
  
  char buffer[30];
  sprintf(buffer, "%d", operand1);
  drawText(buffer, 60, 80, 255, 255, 255, 3);
  
  switch (selectedOption) {
    case 0: drawText("+", 100, 80, 255, 255, 255, 3); break;
    case 1: drawText("-", 100, 80, 255, 255, 255, 3); break;
    case 2: drawText("*", 100, 80, 255, 255, 255, 3); break;
    case 3: drawText("/", 100, 80, 255, 255, 255, 3); break;
  }
  
  sprintf(buffer, "%d", operand2);
  drawText(buffer, 130, 80, 255, 255, 255, 3);
  drawText("=", 60, 130, 255, 255, 255, 3);
  
  // Display instruction for keypad or buttons
  drawText("Enter answer with keypad", 60, 200, 255, 255, 255, 1);
  drawText("ENTER: Submit  BACK: Clear", 60, 220, 255, 255, 255, 1);
}

// Display overlay (e.g., message on screen)
void showOverlay(const char* message, const char* buttonText) {
  int w = SCREEN_WIDTH / 2;
  int h = SCREEN_HEIGHT / 2;
  int x = (SCREEN_WIDTH - w) / 2;
  int y = (SCREEN_HEIGHT - h) / 2;
  
  ucg.setColor(50, 50, 50);
  ucg.drawBox(x, y, w, h);
  
  drawText(message, x + 20, y + 40, 255, 255, 255, 3);
  
  drawText(">>", x + 20, y + 80, 255, 255, 0, 2);
  drawText(buttonText, x + 50, y + 80, 255, 255, 0, 2);
  
  drawText("Press ENTER to continue", x + 20, y + 110, 255, 255, 255, 1);
}

// Check if the user's answer is correct
void checkAnswer() {
  awaitingAnswer = false;
  if (userInput.toInt() == correctAnswer) {
    unsigned long timeTaken = millis() - questionStartTime;
    if (timeTaken < 10000) difficulty++;
    showOverlay("Correct!", "Next");
  } else {
    tone(BUZZER_PIN, 400, 300);  // Using Arduino's tone function instead
    showOverlay("Wrong!", "Retry");
  }
}

// Setup function
void setup() {
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.setRotate90(); // Rotate to landscape mode
  ucg.clearScreen();
  
  // Initialize random seed
  randomSeed(analogRead(0));
  
  // Initialize keypad pins
  for (int i = 0; i < ROWS; i++) {
    pinMode(rowPins[i], INPUT);
  }
  
  for (int i = 0; i < COLS; i++) {
    pinMode(colPins[i], INPUT_PULLDOWN);
  }

  // Initialize joystick pins
  pinMode(JOYSTICK_X_PIN, INPUT);
  pinMode(JOYSTICK_Y_PIN, INPUT);
  pinMode(JOYSTICK_BTN_PIN, INPUT_PULLUP);  // Button is usually active low
  
  // Initialize button pins (all with internal pull-up resistors)
  pinMode(BTN_UP_PIN, INPUT_PULLUP);
  pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
  pinMode(BTN_LEFT_PIN, INPUT_PULLUP);
  pinMode(BTN_RIGHT_PIN, INPUT_PULLUP);  // Using pin 14 now
  pinMode(BTN_ENTER_PIN, INPUT_PULLUP);
  pinMode(BTN_BACK_PIN, INPUT_PULLUP);

  // Start the game
  drawMenu();
}

// Keypad reading function
char getKey() {
  for (int row = 0; row < ROWS; row++) {
    pinMode(rowPins[row], OUTPUT);
    digitalWrite(rowPins[row], HIGH);

    for (int col = 0; col < COLS; col++) {
      pinMode(colPins[col], INPUT_PULLDOWN);
      if (digitalRead(colPins[col]) == HIGH) {
        delay(200); // debounce
        pinMode(rowPins[row], INPUT);
        return keys[row][col];
      }
    }

    digitalWrite(rowPins[row], LOW);
    pinMode(rowPins[row], INPUT);
  }
  return '\0';
}

// Read joystick values and handle joystick input
void handleJoystick() {
  // Read joystick values
  joystickXValue = analogRead(JOYSTICK_X_PIN);
  joystickYValue = analogRead(JOYSTICK_Y_PIN);
  joystickButtonState = !digitalRead(JOYSTICK_BTN_PIN);  // Invert because it's active LOW
  
  unsigned long currentTime = millis();
  
  // Handle joystick button (pressed)
  if (joystickButtonState && !lastJoystickButtonState) {
    if (!awaitingAnswer) {
      // Button acts like "Enter" - start the question
      generateQuestion();
    } else {
      // Button acts like "#" - submit answer
      checkAnswer();
    }
    delay(200); // Debounce
  }
  
  // Only handle joystick movement if enough time has passed
  if (currentTime - lastJoystickMoveTime > joystickDebounceTime) {
    // Handle joystick Y-axis for menu navigation
    if (!awaitingAnswer) {
      if (joystickYValue < 512 - joystickDeadzone) {  // Up
        selectedOption--;
        if (selectedOption < 0) selectedOption = 3;
        drawMenu();
        lastJoystickMoveTime = currentTime;
      } 
      else if (joystickYValue > 512 + joystickDeadzone) {  // Down
        selectedOption++;
        if (selectedOption > 3) selectedOption = 0;
        drawMenu();
        lastJoystickMoveTime = currentTime;
      }
    }
  }
  
  // Save current button state
  lastJoystickButtonState = joystickButtonState;
}

// Read and handle 6-button input
void handleButtons() {
  // Read button states (invert because they're active LOW with pull-ups)
  btnUpState = !digitalRead(BTN_UP_PIN);
  btnDownState = !digitalRead(BTN_DOWN_PIN);
  btnLeftState = !digitalRead(BTN_LEFT_PIN);
  btnRightState = !digitalRead(BTN_RIGHT_PIN);
  btnEnterState = !digitalRead(BTN_ENTER_PIN);
  btnBackState = !digitalRead(BTN_BACK_PIN);
  
  unsigned long currentTime = millis();
  
  // Only process button presses if enough time has passed (debounce)
  if (currentTime - lastButtonPressTime > buttonDebounceTime) {
    
    // Handle UP button
    if (btnUpState && !lastBtnUpState) {
      if (!awaitingAnswer) {
        selectedOption--;
        if (selectedOption < 0) selectedOption = 3;
        drawMenu();
        lastButtonPressTime = currentTime;
      }
    }
    
    // Handle DOWN button
    if (btnDownState && !lastBtnDownState) {
      if (!awaitingAnswer) {
        selectedOption++;
        if (selectedOption > 3) selectedOption = 0;
        drawMenu();
        lastButtonPressTime = currentTime;
      }
    }
    
    // Handle ENTER button
    if (btnEnterState && !lastBtnEnterState) {
      if (!awaitingAnswer) {
        // Start the question
        generateQuestion();
      } else {
        // Submit answer
        checkAnswer();
      }
      lastButtonPressTime = currentTime;
    }
    
    // Handle BACK button
    if (btnBackState && !lastBtnBackState) {
      if (awaitingAnswer) {
        // Clear input
        userInput = "";
        ucg.setColor(0, 0, 0);
        ucg.drawBox(90, 110, 200, 30);
      } else {
        // Go back to menu (if we're not already there)
        if (!awaitingAnswer) {
          drawMenu();
        }
      }
      lastButtonPressTime = currentTime;
    }
    
    // We're not using LEFT/RIGHT buttons for now, but leaving the code for future expansion
  }
  
  // Update last button states
  lastBtnUpState = btnUpState;
  lastBtnDownState = btnDownState;
  lastBtnLeftState = btnLeftState;
  lastBtnRightState = btnRightState;
  lastBtnEnterState = btnEnterState;
  lastBtnBackState = btnBackState;
}

// Handle keypress and update expression
void handleKeypress(char key) {
  if (key == '#') {
    checkAnswer();
  } else if (key == '*') {
    userInput = "";
    // Clear input field by drawing a black rectangle
    ucg.setColor(0, 0, 0);
    ucg.drawBox(90, 110, 200, 30);
  } else if (key >= '0' && key <= '9') {
    userInput += key;
    // Display the updated input
    drawText(userInput.c_str(), 90, 130, 0, 255, 0, 3);  // Green for input text
  }
}

// Main loop
void loop() {
  // Handle keypad input
  char key = getKey();
  if (key != '\0') {
    if (awaitingAnswer) {
      handleKeypress(key);
    } else {
      // When not awaiting answer, A and B are for menu navigation
      if (key == 'A') {
        selectedOption--;
        if (selectedOption < 0) selectedOption = 3;
        drawMenu();
      } else if (key == 'B') {
        selectedOption++;
        if (selectedOption > 3) selectedOption = 0;
        drawMenu();
      } else if (key == '#') {
        // Enter/Select button
        generateQuestion();
      }
    }
  }
  
  // Handle joystick input
  handleJoystick();
  
  // Handle button input
  handleButtons();
}