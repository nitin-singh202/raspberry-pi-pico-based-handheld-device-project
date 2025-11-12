#include <Arduino.h>
#include <Ucglib.h>
#include <SPI.h>

// Buzzer Setup
#define BUZZER_PIN 7

// Display Configuration - adjust these for your specific display
#define TFT_CS   17
#define TFT_DC   15
#define TFT_RST  16

// Create Ucglib display instance for ILI9341
Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

// Display dimensions
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240

// Joystick Pins
#define JOY_X A0
#define JOY_Y A1
#define JOY_SEL 4

int score = 0;
int level = 1;
int gridSize = 3; // Start with 3x3
int selectedX = 0, selectedY = 0;
int prevSelectedX = -1, prevSelectedY = -1;
int cellSize;
bool buttonWasPressed = false;
unsigned long gameStartTime;
int timeLimit = 10000; // Time limit in milliseconds
bool timeChallengeMode = false;
bool invisibleMode = false;
bool rotationMode = false;
int invisibleTimer = 0;
int difficulty = 0; // 0 = Easy, 1 = Medium, 2 = Hard, 3 = Expert, 4 = Impossible

enum ShapeType { CIRCLE, SQUARE, TRIANGLE, LINE_PATTERN, STAR, DIAMOND };
struct Shape {
  ShapeType type;
  uint8_t r, g, b;
  uint8_t size;
  uint8_t pattern;
  int rotation; // For rotated shapes
  bool visible; // For invisible mode
};

Shape grid[5][5]; // Support up to 5x5 grid
Shape oddOne;
int oddX, oddY;

// Colors - store as RGB values
struct RGB {
  uint8_t r, g, b;
};

RGB COLOR_BLACK = {0, 0, 0};
RGB COLOR_WHITE = {255, 255, 255};
RGB COLOR_YELLOW = {255, 255, 0};
RGB COLOR_DARKGREY = {64, 64, 64};
RGB COLOR_RED = {255, 0, 0};
RGB COLOR_GREEN = {0, 255, 0};
RGB COLOR_BLUE = {0, 0, 255};

// ---------- Helper Functions ----------

void buzzerWrong() {
  tone(BUZZER_PIN, 300, 300);
  delay(300);
  noTone(BUZZER_PIN);
}

void buzzerCorrect() {
  tone(BUZZER_PIN, 800, 100);
  delay(110);
  tone(BUZZER_PIN, 1200, 100);
  delay(110);
  noTone(BUZZER_PIN);
}

void drawSplashScreen() {
  ucg.setColor(0, 0, 0);  // BLACK
  ucg.drawBox(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  
  ucg.setFont(ucg_font_helvR18_tr);  
  ucg.setColor(255, 255, 255);  // WHITE
  ucg.setPrintPos(40, 100);
  ucg.print("Odd One Out Game");
  
  ucg.setFont(ucg_font_helvR12_tr);
  ucg.setPrintPos(80, 140);
  ucg.print("Press to Start");
  
  // Wait for button press
  while (digitalRead(JOY_SEL) == HIGH) {
    delay(10);
  }
  delay(200); // Debounce
}

Shape randomShape() {
  Shape s;
  s.type = static_cast<ShapeType>(random(0, 6)); // Including STAR and DIAMOND
  s.r = random(50, 256);
  s.g = random(50, 256);
  s.b = random(50, 256);
  s.size = 20 + random(0, 10);
  s.pattern = random(0, 2);
  s.rotation = 0;
  s.visible = true;
  return s;
}

void drawStar(int centerX, int centerY, int size) {
  // Draw a 5-pointed star
  for (int i = 0; i < 5; i++) {
    int angle1 = i * 72;
    int angle2 = (i + 2) * 72;
    
    int x1 = centerX + (size * cos(angle1 * PI / 180));
    int y1 = centerY + (size * sin(angle1 * PI / 180));
    int x2 = centerX + (size * cos(angle2 * PI / 180));
    int y2 = centerY + (size * sin(angle2 * PI / 180));
    
    ucg.drawLine(x1, y1, x2, y2);
  }
}

void drawDiamond(int centerX, int centerY, int size) {
  // Draw a diamond shape
  int halfSize = size / 2;
  ucg.drawLine(centerX, centerY - halfSize, centerX + halfSize, centerY);
  ucg.drawLine(centerX + halfSize, centerY, centerX, centerY + halfSize);
  ucg.drawLine(centerX, centerY + halfSize, centerX - halfSize, centerY);
  ucg.drawLine(centerX - halfSize, centerY, centerX, centerY - halfSize);
}

void drawShape(int x, int y, const Shape& s, bool highlight = false) {
  int centerX = x * cellSize + cellSize / 2;
  int centerY = y * cellSize + cellSize / 2;
  
  // Background clear only if necessary
  if (highlight != (x == prevSelectedX && y == prevSelectedY)) {
    ucg.setColor(0, 0, 0);  // BLACK
    ucg.drawBox(x * cellSize, y * cellSize, cellSize, cellSize);
  }
  
  // Check if shape should be visible
  if (invisibleMode && !s.visible && !highlight) {
    return; // Don't draw invisible shapes unless highlighted
  }
  
  // Set shape color or highlight color
  if (highlight) {
    ucg.setColor(255, 255, 0);  // YELLOW
  } else {
    ucg.setColor(s.r, s.g, s.b);
  }

  // Apply rotation if needed
  if (rotationMode && s.rotation > 0) {
    // Simple rotation effect using offset
    centerX += s.rotation * 2;
    centerY += s.rotation * 2;
  }

  switch (s.type) {
    case CIRCLE:
      ucg.drawDisc(centerX, centerY, s.size / 2, UCG_DRAW_ALL);
      break;
    case SQUARE:
      ucg.drawBox(centerX - s.size / 2, centerY - s.size / 2, s.size, s.size);
      break;
    case TRIANGLE:
      {
        int x1 = centerX;
        int y1 = centerY - s.size / 2;
        int x2 = centerX - s.size / 2;
        int y2 = centerY + s.size / 2;
        int x3 = centerX + s.size / 2;
        int y3 = centerY + s.size / 2;
        
        ucg.drawTriangle(x1, y1, x2, y2, x3, y3);
      }
      break;
    case LINE_PATTERN:
      for (int i = 0; i < s.size; i += 3) {
        ucg.drawVLine(centerX - s.size / 2 + i, centerY - s.size / 2, s.size);
      }
      break;
    case STAR:
      drawStar(centerX, centerY, s.size / 2);
      break;
    case DIAMOND:
      drawDiamond(centerX, centerY, s.size);
      break;
  }

  // Highlight Box
  if (highlight) {
    ucg.setColor(255, 255, 255);  // WHITE
    ucg.drawFrame(x * cellSize, y * cellSize, cellSize, cellSize);
  }
}

void updateDifficulty() {
  // Increase difficulty based on score
  if (score < 5) {
    difficulty = 0; // Easy
    gridSize = 3;
    timeChallengeMode = false;
    invisibleMode = false;
    rotationMode = false;
  } else if (score < 10) {
    difficulty = 1; // Medium
    gridSize = 3;
    timeChallengeMode = true;
    timeLimit = 10000;
    invisibleMode = false;
    rotationMode = false;
  } else if (score < 15) {
    difficulty = 2; // Hard
    gridSize = 4;
    timeChallengeMode = true;
    timeLimit = 8000;
    invisibleMode = false;
    rotationMode = true;
  } else if (score < 20) {
    difficulty = 3; // Expert
    gridSize = 4;
    timeChallengeMode = true;
    timeLimit = 6000;
    invisibleMode = true;
    rotationMode = true;
  } else {
    difficulty = 4; // Impossible
    gridSize = 5;
    timeChallengeMode = true;
    timeLimit = 5000;
    invisibleMode = true;
    rotationMode = true;
  }
  
  // Recalculate cell size
  cellSize = min(DISPLAY_WIDTH / gridSize, DISPLAY_HEIGHT / gridSize);
}

void generateGrid() {
  Shape base = randomShape();
  oddX = random(0, gridSize);
  oddY = random(0, gridSize);

  for (int y = 0; y < gridSize; y++) {
    for (int x = 0; x < gridSize; x++) {
      grid[y][x] = base;
      grid[y][x].visible = true;
    }
  }

  // Change one attribute for the odd shape
  Shape modified = base;
  int changeType = random(0, difficulty + 3); // More options at higher difficulty
  
  switch (changeType) {
    case 0: // Color change
      modified.r = random(50, 256);
      modified.g = random(50, 256);
      modified.b = random(50, 256);
      break;
    case 1: // Size change
      modified.size += (difficulty == 4) ? 5 : 10; // Smaller difference at highest difficulty
      break;
    case 2: // Pattern change
      modified.pattern = (modified.pattern == 0) ? 1 : 0;
      break;
    case 3: // Shape change
      modified.type = static_cast<ShapeType>((modified.type + 1) % 6);
      break;
    case 4: // Rotation change (only at harder difficulties)
      modified.rotation = 15;
      break;
    default: // Combined changes (only at expert/impossible)
      modified.r += 30;
      modified.size += 5;
      modified.type = static_cast<ShapeType>((modified.type + 1) % 6);
      break;
  }

  grid[oddY][oddX] = modified;
  oddOne = modified;
  
  // Handle invisible mode
  if (invisibleMode) {
    for (int y = 0; y < gridSize; y++) {
      for (int x = 0; x < gridSize; x++) {
        // Make some shapes invisible, but never the odd one
        if (!(x == oddX && y == oddY) && random(0, 3) == 0) {
          grid[y][x].visible = false;
        }
      }
    }
  }
  
  gameStartTime = millis();
}

void drawGrid() {
  // Clear the screen only when necessary
  if (prevSelectedX == -1 || prevSelectedY == -1) {
    ucg.setColor(0, 0, 0);  // BLACK
    ucg.drawBox(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  }
  
  // Display difficulty level
  const char* difficultyText[] = {"Easy", "Medium", "Hard", "Expert", "Impossible"};
  ucg.setFont(ucg_font_helvR10_tr);
  ucg.setColor(255, 255, 255);  // WHITE
  ucg.setPrintPos(5, 15);
  ucg.print("Level: ");
  ucg.print(difficultyText[difficulty]);
  
  // Score
  ucg.setPrintPos(5, 30);
  ucg.print("Score: ");
  ucg.print(score);
  
  // Time remaining (if in time challenge mode)
  if (timeChallengeMode) {
    unsigned long elapsedTime = millis() - gameStartTime;
    int remainingTime = (timeLimit - elapsedTime) / 1000;
    
    ucg.setPrintPos(200, 15);
    if (remainingTime <= 3) {
      ucg.setColor(255, 0, 0);  // RED
    } else {
      ucg.setColor(255, 255, 255);  // WHITE
    }
    ucg.print("Time: ");
    ucg.print(remainingTime);
    ucg.print("s");
  }
  
  // Draw shapes
  if (prevSelectedX == -1 || prevSelectedY == -1) {
    // Draw all shapes
    for (int y = 0; y < gridSize; y++) {
      for (int x = 0; x < gridSize; x++) {
        drawShape(x, y, grid[y][x], (x == selectedX && y == selectedY));
      }
    }
  } else {
    // Only redraw the changed cells
    if (prevSelectedX != selectedX || prevSelectedY != selectedY) {
      // Redraw previous selected cell
      drawShape(prevSelectedX, prevSelectedY, grid[prevSelectedY][prevSelectedX], false);
      // Draw new selected cell
      drawShape(selectedX, selectedY, grid[selectedY][selectedX], true);
    }
  }
  
  prevSelectedX = selectedX;
  prevSelectedY = selectedY;
}

void showOverlay(const char* message, const char* option) {
  int overlayW = 200;
  int overlayH = 120;
  int x = (DISPLAY_WIDTH - overlayW) / 2;
  int y = (DISPLAY_HEIGHT - overlayH) / 2;
  
  ucg.setColor(64, 64, 64);  // DARKGREY
  ucg.drawBox(x, y, overlayW, overlayH);
  ucg.setColor(255, 255, 255);  // WHITE
  ucg.drawFrame(x, y, overlayW, overlayH);

  ucg.setFont(ucg_font_helvR14_tr);
  ucg.setColor(255, 255, 255);  // WHITE
  ucg.setPrintPos(x + 10, y + 40);
  ucg.print(message);

  // Show score
  ucg.setPrintPos(x + 10, y + 65);
  ucg.print("Score: ");
  ucg.print(score);

  ucg.setColor(255, 255, 0);  // YELLOW
  ucg.setPrintPos(x + 10, y + 95);
  ucg.print(">> ");
  ucg.print(option);
}

void handleSelection() {
  if (timeChallengeMode) {
    unsigned long elapsedTime = millis() - gameStartTime;
    if (elapsedTime > timeLimit) {
      buzzerWrong();
      showOverlay("Time's Up!", "Retry");
      delay(1500);
      generateGrid();
      prevSelectedX = -1;
      prevSelectedY = -1;
      drawGrid();
      return;
    }
  }
  
  if (selectedX == oddX && selectedY == oddY) {
    score++;
    buzzerCorrect();
    showOverlay("Correct!", "Next");
    delay(1000);
    updateDifficulty();
  } else {
    buzzerWrong();
    showOverlay("Wrong!", "Retry");
    delay(1500);
  }

  generateGrid();
  prevSelectedX = -1;
  prevSelectedY = -1;
  drawGrid();
}

// ---------- Input Handling ----------

void handleJoystick() {
  int xVal = analogRead(JOY_X);
  int yVal = analogRead(JOY_Y);

  static unsigned long lastMove = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastMove > 150) {
    bool moved = false;
    
    if (xVal < 400 && selectedX > 0) {
      selectedX--;
      moved = true;
    } else if (xVal > 600 && selectedX < gridSize - 1) {
      selectedX++;
      moved = true;
    }
    if (yVal < 400 && selectedY > 0) {
      selectedY--;
      moved = true;
    } else if (yVal > 600 && selectedY < gridSize - 1) {
      selectedY++;
      moved = true;
    }
    
    if (moved) {
      lastMove = currentTime;
      drawGrid();
    }
  }

  // Handle button press with debouncing
  bool buttonPressed = (digitalRead(JOY_SEL) == LOW);
  
  if (buttonPressed && !buttonWasPressed) {
    handleSelection();
    buttonWasPressed = true;
  } else if (!buttonPressed && buttonWasPressed) {
    buttonWasPressed = false;
  }
}

// ---------- Main Setup and Loop ----------

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(JOY_SEL, INPUT_PULLUP);
  
  // Initialize SPI for display
  SPI.begin();
  delay(50);
  
  // Initialize the display
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.setRotate90();
  
  randomSeed(analogRead(A0)); // Initialize random seed
  
  drawSplashScreen();
  updateDifficulty();
  generateGrid();
  drawGrid();
}

void loop() {
  handleJoystick();
  
  // Check for time expiration in time challenge mode
  if (timeChallengeMode) {
    unsigned long elapsedTime = millis() - gameStartTime;
    if (elapsedTime > timeLimit) {
      buzzerWrong();
      showOverlay("Time's Up!", "Retry");
      delay(1500);
      generateGrid();
      prevSelectedX = -1;
      prevSelectedY = -1;
      drawGrid();
    }
  }
}