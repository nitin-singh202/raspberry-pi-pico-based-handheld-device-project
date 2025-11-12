#include <Arduino.h>
#include <Ucglib.h>
#include <SPI.h>
#include <Keypad.h>  // Add Keypad library

// Display configuration - native resolution is 240×320 (portrait)
// But we'll use it in landscape orientation (320×240)
#define SCREEN_HEIGHT 240  // In landscape, this is the width
#define SCREEN_WIDTH 320   // In landscape, this is the height
#define TFT_CS   17
#define TFT_DC   15
#define TFT_RST  16
#define BUZZER_PIN 5

// Joystick/Input pins
#define JOY_X_PIN A0
#define JOY_Y_PIN A1
#define JOY_BTN_PIN 4

// Keypad configuration - 4x4 matrix keypad
const byte KEYPAD_ROWS = 4; 
const byte KEYPAD_COLS = 4;
// Define the keypad layout (adjust as needed for your specific keypad)
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1', '2', '3', 'A'},  // A can be used for special functions
  {'4', '5', '6', 'B'},  // B can be used for special functions 
  {'7', '8', '9', 'C'},  // C can be used for special functions
  {'*', '0', '#', 'D'}   // * and # and D can be used for special functions
};
// Connect keypad ROW0, ROW1, ROW2, ROW3 to these Arduino pins
byte colPins[KEYPAD_ROWS] = {13, 12, 11, 10}; // Change these pins as needed
// Connect keypad COL0, COL1, COL2, COL3 to these Arduino pins
byte rowPins[KEYPAD_COLS] = {9, 8, 7, 6}; // Change these pins as needed

// Create the Keypad instance
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

// Create UCGLib instance for ILI9341 
// This is for a 240×320 display used in landscape orientation
Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

// Colors
#define BLACK       0, 0, 0
#define WHITE       255, 255, 255
#define LIGHT_GREY  200, 200, 200
#define DARK_GREY   80, 80, 80
#define BLUE        0, 0, 255
#define HIGHLIGHT   0, 150, 255
#define PRESET_NUM  50, 100, 50    // Green for preset numbers
#define USER_NUM    200, 200, 0    // Yellow for user input

// Game variables
#define GRID_SIZE 9
#define BOX_SIZE 3
int sudokuGrid[GRID_SIZE][GRID_SIZE];
bool fixedCells[GRID_SIZE][GRID_SIZE];
int selectedRow = 0, selectedCol = 0;
bool gameComplete = false;

// Display layout variables
int cellSize;
int gridMarginX, gridMarginY;

// Buffer for previous state to minimize screen updates
int previousGrid[GRID_SIZE][GRID_SIZE];
int previousSelectedRow = -1, previousSelectedCol = -1;

// Function to update only changed cells
void updateCell(int row, int col) {
  int x = gridMarginX + (col * cellSize);
  int y = gridMarginY + (row * cellSize);
  
  // Clear cell background
  if (row == selectedRow && col == selectedCol) {
    ucg.setColor(HIGHLIGHT);
  } else {
    ucg.setColor(WHITE);
  }
  ucg.drawBox(x + 1, y + 1, cellSize - 1, cellSize - 1);
  
  // Draw number if cell is not empty
  if (sudokuGrid[row][col] != 0) {
    // Set color based on whether it's a preset number or user input
    if (fixedCells[row][col]) {
      ucg.setColor(PRESET_NUM);
    } else {
      ucg.setColor(USER_NUM);
    }
    
    ucg.setFont(ucg_font_inr16_mr);
    
    // Calculate position to center text
    int textWidth = 8;  // Approximate width of a digit with the selected font
    int textHeight = 12;  // Approximate height with the selected font
    
    int textX = x + (cellSize - textWidth) / 2;
    int textY = y + (cellSize + textHeight) / 2;
    
    ucg.setPrintPos(textX, textY);
    ucg.print(sudokuGrid[row][col]);
  }
}

// Draws the entire Sudoku grid with current state
void drawGrid() {
  // Check if this is the first draw or a full redraw is needed
  static bool firstDraw = true;
  
  if (firstDraw) {
    // Clear screen
    ucg.setColor(BLACK);
    ucg.drawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // Calculate cell size to optimize for landscape layout
    cellSize = min((SCREEN_WIDTH - 40) / GRID_SIZE, (SCREEN_HEIGHT - 40) / GRID_SIZE);
    
    // Center the grid on screen
    gridMarginX = (SCREEN_WIDTH - (cellSize * GRID_SIZE)) / 2;
    gridMarginY = (SCREEN_HEIGHT - (cellSize * GRID_SIZE)) / 2;
    
    // Draw the grid background
    ucg.setColor(WHITE);
    ucg.drawBox(gridMarginX, gridMarginY, cellSize * GRID_SIZE, cellSize * GRID_SIZE);
    
    // Draw grid lines
    for (int i = 0; i <= GRID_SIZE; i++) {
      // Determine line thickness - thicker for box boundaries
      int thickness = (i % BOX_SIZE == 0) ? 2 : 1;
      
      // Vertical lines
      if (i % BOX_SIZE == 0) {
        ucg.setColor(BLACK);  // Bold lines for 3x3 boxes
      } else {
        ucg.setColor(DARK_GREY);  // Regular lines between cells
      }
      
      for (int t = 0; t < thickness; t++) {
        int x = gridMarginX + (i * cellSize) - t;
        ucg.drawVLine(x, gridMarginY, cellSize * GRID_SIZE);
      }
      
      // Horizontal lines
      if (i % BOX_SIZE == 0) {
        ucg.setColor(BLACK);  // Bold lines for 3x3 boxes
      } else {
        ucg.setColor(DARK_GREY);  // Regular lines between cells
      }
      
      for (int t = 0; t < thickness; t++) {
        int y = gridMarginY + (i * cellSize) - t;
        ucg.drawHLine(gridMarginX, y, cellSize * GRID_SIZE);
      }
    }
    
    // Display game status at the bottom - optimized for landscape
    ucg.setColor(WHITE);
    ucg.setFont(ucg_font_inr16_mr);
    ucg.setPrintPos(10, SCREEN_HEIGHT - 10);
    ucg.print("Move: Joy | Enter: 1-9 | C: Clear");
    
    firstDraw = false;
  }
  
  // Update only cells that have changed
  for (int row = 0; row < GRID_SIZE; row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      if (sudokuGrid[row][col] != previousGrid[row][col] || 
          (row == selectedRow && col == selectedCol) || 
          (row == previousSelectedRow && col == previousSelectedCol)) {
        updateCell(row, col);
        previousGrid[row][col] = sudokuGrid[row][col];
      }
    }
  }
  
  previousSelectedRow = selectedRow;
  previousSelectedCol = selectedCol;
}

// Initialize a new Sudoku puzzle
void generateSudoku() {
  // Clear the grid
  memset(sudokuGrid, 0, sizeof(sudokuGrid));
  memset(fixedCells, 0, sizeof(fixedCells));
  memset(previousGrid, -1, sizeof(previousGrid));  // Reset previous grid
  previousSelectedRow = -1;
  previousSelectedCol = -1;
  
  // Create a basic pattern (This should be replaced with a proper Sudoku generator)
  int example[9][9] = {
    {5, 3, 0, 0, 7, 0, 0, 0, 0},
    {6, 0, 0, 1, 9, 5, 0, 0, 0},
    {0, 9, 8, 0, 0, 0, 0, 6, 0},
    {8, 0, 0, 0, 6, 0, 0, 0, 3},
    {4, 0, 0, 8, 0, 3, 0, 0, 1},
    {7, 0, 0, 0, 2, 0, 0, 0, 6},
    {0, 6, 0, 0, 0, 0, 2, 8, 0},
    {0, 0, 0, 4, 1, 9, 0, 0, 5},
    {0, 0, 0, 0, 8, 0, 0, 7, 9}
  };
  
  // Copy to our game grid and mark fixed cells
  for (int row = 0; row < GRID_SIZE; row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      sudokuGrid[row][col] = example[row][col];
      if (example[row][col] != 0) {
        fixedCells[row][col] = true;
      }
    }
  }
  
  selectedRow = 0;
  selectedCol = 0;
  gameComplete = false;
}

// Check if a number can be placed at the given position
bool isValidMove(int row, int col, int num) {
  // Check row
  for (int c = 0; c < GRID_SIZE; c++) {
    if (c != col && sudokuGrid[row][c] == num) return false;
  }
  
  // Check column
  for (int r = 0; r < GRID_SIZE; r++) {
    if (r != row && sudokuGrid[r][col] == num) return false;
  }
  
  // Check 3x3 box
  int boxRow = row - (row % BOX_SIZE);
  int boxCol = col - (col % BOX_SIZE);
  
  for (int r = 0; r < BOX_SIZE; r++) {
    for (int c = 0; c < BOX_SIZE; c++) {
      int currentR = boxRow + r;
      int currentC = boxCol + c;
      if (currentR != row || currentC != col) {
        if (sudokuGrid[currentR][currentC] == num) return false;
      }
    }
  }
  
  return true;
}

// Process a number input from keypad
void processNumberInput(char key) {
  if (!fixedCells[selectedRow][selectedCol]) {
    // Convert char to int (ASCII '1' -> 1, etc.)
    int num = key - '0';
    
    // Only set the value if it's a valid move and within range 1-9
    if (num >= 1 && num <= 9) {
      if (isValidMove(selectedRow, selectedCol, num)) {
        sudokuGrid[selectedRow][selectedCol] = num;
        
        // Play confirmation sound
        tone(BUZZER_PIN, 800, 50);
      } else {
        // Feedback for invalid move
        tone(BUZZER_PIN, 200, 100);  // Short low beep
      }
      
      drawGrid();  // Redraw with new number
    }
  } else {
    // Feedback for trying to modify a fixed cell
    tone(BUZZER_PIN, 150, 200);
  }
}

// Handle both joystick and keypad input
void handleInput() {
  // Check for keypad input first
  char key = keypad.getKey();
  if (key) {
    // If a key was pressed
    switch (key) {
      case '1': case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9':
        processNumberInput(key);
        break;
      
      case 'C': // Clear cell
        if (!fixedCells[selectedRow][selectedCol]) {
          sudokuGrid[selectedRow][selectedCol] = 0;
          drawGrid();
          tone(BUZZER_PIN, 600, 50);  // Confirmation beep
        } else {
          tone(BUZZER_PIN, 150, 200);  // Error beep
        }
        break;
        
      case 'A': // Move up
        if (selectedRow > 0) {
          selectedRow--;
          drawGrid();
          tone(BUZZER_PIN, 400, 20);
        }
        break;
        
      case 'B': // Move down
        if (selectedRow < GRID_SIZE - 1) {
          selectedRow++;
          drawGrid();
          tone(BUZZER_PIN, 400, 20);
        }
        break;
        
      case '*': // Move left
        if (selectedCol > 0) {
          selectedCol--;
          drawGrid();
          tone(BUZZER_PIN, 400, 20);
        }
        break;
        
      case '#': // Move right
        if (selectedCol < GRID_SIZE - 1) {
          selectedCol++;
          drawGrid();
          tone(BUZZER_PIN, 400, 20);
        }
        break;
        
      case 'D': // Check solution
        if (isSudokuComplete()) {
          gameComplete = true;
          showCompletionMessage();
        } else {
          // Show incomplete message
          tone(BUZZER_PIN, 200, 200);
        }
        break;
    }
  }
  
  // Read joystick inputs for movement
  int joyX = analogRead(JOY_X_PIN);
  int joyY = analogRead(JOY_Y_PIN);
  bool buttonPressed = !digitalRead(JOY_BTN_PIN);  // Assuming active low
  
  static unsigned long lastMoveTime = 0;
  static unsigned long lastButtonTime = 0;
  
  // Check for joystick movement with debounce
  if (millis() - lastMoveTime > 200) {  // 200ms debounce
    bool moved = false;
    
    if (joyX < 300) {  // Left
      if (selectedCol > 0) {
        selectedCol--;
        moved = true;
      }
    } else if (joyX > 700) {  // Right
      if (selectedCol < GRID_SIZE - 1) {
        selectedCol++;
        moved = true;
      }
    }
    
    if (joyY < 300) {  // Up
      if (selectedRow > 0) {
        selectedRow--;
        moved = true;
      }
    } else if (joyY > 700) {  // Down
      if (selectedRow < GRID_SIZE - 1) {
        selectedRow++;
        moved = true;
      }
    }
    
    if (moved) {
      lastMoveTime = millis();
      drawGrid();  // Redraw to show new selection
      tone(BUZZER_PIN, 300, 20); // Movement sound
    }
  }
  
  // Handle joystick button press with debounce 
  if (buttonPressed && (millis() - lastButtonTime > 300)) {
    lastButtonTime = millis();
    
    // Cycle through numbers 1-9 on button press (if cell isn't fixed)
    if (!fixedCells[selectedRow][selectedCol]) {
      int currentValue = sudokuGrid[selectedRow][selectedCol];
      int newValue = (currentValue % 9) + 1;
      
      // Only set the value if it's a valid move
      if (isValidMove(selectedRow, selectedCol, newValue)) {
        sudokuGrid[selectedRow][selectedCol] = newValue;
        tone(BUZZER_PIN, 800, 50);  // Confirmation beep
      } else {
        // Feedback for invalid move
        tone(BUZZER_PIN, 200, 100);  // Short low beep
      }
      
      drawGrid();  // Redraw with new number
    } else {
      // Feedback for trying to modify a fixed cell
      tone(BUZZER_PIN, 150, 200);
    }
  }
}

// Check if the Sudoku is completely filled and valid
bool isSudokuComplete() {
  // First check if all cells are filled
  for (int row = 0; row < GRID_SIZE; row++) {
    for (int col = 0; col < GRID_SIZE; col++) {
      if (sudokuGrid[row][col] == 0) return false;
    }
  }
  
  // Check rows for validity
  for (int row = 0; row < GRID_SIZE; row++) {
    bool found[GRID_SIZE + 1] = {false};
    for (int col = 0; col < GRID_SIZE; col++) {
      int num = sudokuGrid[row][col];
      if (num < 1 || num > 9 || found[num]) return false;
      found[num] = true;
    }
  }
  
  // Check columns for validity
  for (int col = 0; col < GRID_SIZE; col++) {
    bool found[GRID_SIZE + 1] = {false};
    for (int row = 0; row < GRID_SIZE; row++) {
      int num = sudokuGrid[row][col];
      if (num < 1 || num > 9 || found[num]) return false;
      found[num] = true;
    }
  }
  
  // Check 3x3 boxes for validity
  for (int boxRow = 0; boxRow < GRID_SIZE; boxRow += BOX_SIZE) {
    for (int boxCol = 0; boxCol < GRID_SIZE; boxCol += BOX_SIZE) {
      bool found[GRID_SIZE + 1] = {false};
      
      for (int r = 0; r < BOX_SIZE; r++) {
        for (int c = 0; c < BOX_SIZE; c++) {
          int num = sudokuGrid[boxRow + r][boxCol + c];
          if (num < 1 || num > 9 || found[num]) return false;
          found[num] = true;
        }
      }
    }
  }
  
  return true;
}

// Display completion message optimized for landscape
void showCompletionMessage() {
  // Create a centered message box
  int boxWidth = SCREEN_WIDTH / 2;
  int boxHeight = SCREEN_HEIGHT / 3;
  int boxX = (SCREEN_WIDTH - boxWidth) / 2;
  int boxY = (SCREEN_HEIGHT - boxHeight) / 2;
  
  ucg.setColor(0, 0, 100);  // Dark blue
  ucg.drawBox(boxX, boxY, boxWidth, boxHeight);
  ucg.setColor(255, 255, 255);
  ucg.drawFrame(boxX, boxY, boxWidth, boxHeight);
  
  // Center the text better
  ucg.setFont(ucg_font_inr19_mr);
  ucg.setPrintPos(boxX + (boxWidth/2) - 50, boxY + 40);
  ucg.print("Completed!");
  
  ucg.setFont(ucg_font_inr16_mr);
  ucg.setPrintPos(boxX + 20, boxY + 70);
  ucg.print("Press button for new game");
  
  // Play victory tune
  tone(BUZZER_PIN, 523, 100);  // C5
  delay(150);
  tone(BUZZER_PIN, 659, 100);  // E5
  delay(150);
  tone(BUZZER_PIN, 784, 150);  // G5
  delay(200);
  tone(BUZZER_PIN, 1047, 400); // C6
}

// Display welcome screen optimized for landscape
void showWelcomeScreen() {
  ucg.setColor(0, 0, 0);
  ucg.drawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  
  // Title centered in landscape
  ucg.setColor(255, 255, 255);
  ucg.setFont(ucg_font_inr24_mr);
  ucg.setPrintPos((SCREEN_WIDTH - 120) / 2, 80);
  ucg.print("9x9 Sudoku");
  
  // Instructions centered in landscape
  ucg.setFont(ucg_font_inr16_mr);
  ucg.setPrintPos((SCREEN_WIDTH - 240) / 2, 120);
  ucg.print("Controls:");
  
  ucg.setPrintPos((SCREEN_WIDTH - 240) / 2, 145);
  ucg.print("- Joystick/A,B,*,# to move");
  
  ucg.setPrintPos((SCREEN_WIDTH - 240) / 2, 170);
  ucg.print("- Keys 1-9 to enter numbers");
  
  ucg.setPrintPos((SCREEN_WIDTH - 240) / 2, 195);
  ucg.print("- C to clear, D to check");
  
  ucg.setPrintPos((SCREEN_WIDTH - 200) / 2, 230);
  ucg.print("Press any key to start");
}

void setup() {
  Serial.begin(115200);
  Serial.println("9x9 Sudoku starting in landscape mode with keypad...");
  
  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(JOY_BTN_PIN, INPUT_PULLUP);
  
  // Initialize SPI and display
  SPI.begin();
  delay(50);
  
  // Initialize the display with rotation to achieve landscape mode
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  
  // Set the display to landscape orientation (90° rotation)
  ucg.setRotate90();  // Rotate display 90 degrees for landscape
  
  ucg.clearScreen();
  delay(100);
  
  // Play startup sound
  tone(BUZZER_PIN, 262, 100);  // C4
  delay(100);
  tone(BUZZER_PIN, 330, 100);  // E4
  delay(100);
  tone(BUZZER_PIN, 392, 150);  // G4
  
  // Display welcome screen
  showWelcomeScreen();
  
  // Wait for button press or any keypress
  bool startGame = false;
  while (!startGame) {
    if (!digitalRead(JOY_BTN_PIN) || keypad.getKey()) {
      startGame = true;
    }
    delay(50);
  }
  
  // Initialize game
  generateSudoku();
  drawGrid();
}

void loop() {
  if (!gameComplete) {
    handleInput();
    
    // Check for completion
    if (isSudokuComplete()) {
      gameComplete = true;
      showCompletionMessage();
    }
  } else {
    // Start new game if button pressed or key pressed while complete
    if (!digitalRead(JOY_BTN_PIN) || keypad.getKey()) {
      delay(300);  // Debounce
      generateSudoku();
      drawGrid();
      gameComplete = false;
    }
  }
  
  delay(20);  // Small delay for stability
}