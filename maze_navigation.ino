#ifndef MAZE_GAME_H
#define MAZE_GAME_H

#include <Ucglib.h>

// Display configuration
#define TFT_CS   17
#define TFT_DC   15
#define TFT_RST  16
#define BUZZER_PIN 0

// Create display instance with hardware SPI
Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

// Joystick configuration
#define JOY_X A0
#define JOY_Y A1
#define JOY_BTN 5

// Screen parameters
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BLOCK_SIZE 10

// Color definitions
#define COLOR_BLACK 0, 0, 0
#define COLOR_WHITE 255, 255, 255
#define COLOR_RED 255, 0, 0
#define COLOR_GREEN 0, 255, 0
#define COLOR_BLUE 0, 0, 255
#define COLOR_YELLOW 255, 255, 0
#define COLOR_MAGENTA 255, 0, 255

// Maze parameters
#define MAZE_WIDTH 32
#define MAZE_HEIGHT 22  // Reduced height to leave room for title

// Game variables
int playerX, playerY;
int goalX, goalY;
bool gameWon = false;
const int moveDelay = 150;
unsigned long lastMove = 0;
int currentLevel = 1;

// Button handling variables
bool buttonPressed = false;
unsigned long lastButtonCheck = 0;
const int buttonDebounceTime = 200; // milliseconds

// 0 = path, 1 = wall, 2 = goal
int maze[MAZE_HEIGHT][MAZE_WIDTH];

// Function prototypes
void generateMaze();
void drawMaze();
void handleJoystick();
void checkJoystickButton();
void showWinScreen();
void resetGame();
void drawTitle();

// Draw a single block
void drawBlock(int x, int y, const uint8_t r, const uint8_t g, const uint8_t b) {
  ucg.setColor(r, g, b);
  ucg.drawBox(x, y, BLOCK_SIZE, BLOCK_SIZE);
}

// Draw the title at the top of the screen
void drawTitle() {
  ucg.setColor(COLOR_BLUE);
  ucg.drawBox(0, 0, SCREEN_WIDTH, 20);
  ucg.setColor(COLOR_WHITE);
  ucg.setFont(ucg_font_ncenR12_tr);
  ucg.setPrintPos((SCREEN_WIDTH - 16 * 15) / 2, 15);
  ucg.print("MAZE NAVIGATION");
  
  // Display level
  ucg.setPrintPos(SCREEN_WIDTH - 80, 15);
  ucg.print("Lvl:");
  ucg.print(currentLevel);
}

// Generate a procedural maze using depth-first search algorithm
void generateMaze() {
  // Initialize maze with all walls
  for (int y = 0; y < MAZE_HEIGHT; y++) {
    for (int x = 0; x < MAZE_WIDTH; x++) {
      maze[y][x] = 1; // 1 = wall
    }
  }
  
  struct Cell { int x, y; };
  Cell stack[MAZE_WIDTH * MAZE_HEIGHT];
  int stackSize = 0;
  
  int dx[] = {0, 0, -1, 1};
  int dy[] = {-1, 1, 0, 0};
  
  // Start at position (1,1) to ensure walls around the edge
  int cx = 1, cy = 1;
  maze[cy][cx] = 0; // 0 = path
  stack[stackSize++] = {cx, cy};
  
  // FIX: Keep consistent difficulty across levels 1 and 2
  // Instead of increasing skipRate, use different maze generation parameters
  // that maintain the same difficulty level
  int skipRate = 0; // Always use 0 to ensure consistent path generation
  
  // Use level to seed different maze patterns
  randomSeed(analogRead(A2) + currentLevel * 100);
  
  while (stackSize > 0) {
    Cell current = stack[stackSize - 1];
    cx = current.x;
    cy = current.y;
    
    // Randomize direction order
    int dirs[4] = {0, 1, 2, 3};
    for (int i = 0; i < 4; i++) {
      int r = random(4);
      int tmp = dirs[i];
      dirs[i] = dirs[r];
      dirs[r] = tmp;
    }
    
    bool moved = false;
    for (int i = 0; i < 4; i++) {
      int nx = cx + dx[dirs[i]] * 2;
      int ny = cy + dy[dirs[i]] * 2;
      
      // Check if the new position is within bounds and is a wall
      if (nx > 0 && ny > 0 && nx < MAZE_WIDTH - 1 && ny < MAZE_HEIGHT - 1 && maze[ny][nx] == 1) {
        // Make a path - keeping consistent generation across levels
        maze[cy + dy[dirs[i]]][cx + dx[dirs[i]]] = 0;
        maze[ny][nx] = 0;
        
        // For level 2, we'll add some random extra paths (but maintain similar difficulty)
        if (currentLevel == 2 && random(20) == 0) {
          // Add an occasional extra path for visual variety
          int randDir = random(4);
          int extraX = nx + dx[randDir];
          int extraY = ny + dy[randDir];
          if (extraX > 0 && extraY > 0 && extraX < MAZE_WIDTH - 1 && extraY < MAZE_HEIGHT - 1) {
            maze[extraY][extraX] = 0;
          }
        }
        
        stack[stackSize++] = {nx, ny};
        moved = true;
        break;
      }
    }
    
    if (!moved) {
      stackSize--;
    }
  }
  
  // Set start position (player)
  playerX = 1 * BLOCK_SIZE;
  playerY = 1 * BLOCK_SIZE;
  maze[1][1] = 0;
  
  // Set goal position
  int goalGridX = MAZE_WIDTH - 2;
  int goalGridY = MAZE_HEIGHT - 2;
  maze[goalGridY][goalGridX] = 2;
  goalX = goalGridX * BLOCK_SIZE;
  goalY = goalGridY * BLOCK_SIZE;
  
  // Ensure there's a path to the goal
  // This is a simple approach; for complex mazes, a pathfinding algorithm would be better
  maze[goalGridY][goalGridX - 1] = 0;
  maze[goalGridY - 1][goalGridX] = 0;
  
  // For level 2, add a few more decorative elements without changing difficulty
  if (currentLevel == 2) {
    // Add a few random "islands" (isolated wall blocks) in open areas
    for (int i = 0; i < 10; i++) {
      int rx = random(3, MAZE_WIDTH - 3);
      int ry = random(3, MAZE_HEIGHT - 3);
      // Only place if surrounded by paths to avoid blocking routes
      if (maze[ry-1][rx] == 0 && maze[ry+1][rx] == 0 && 
          maze[ry][rx-1] == 0 && maze[ry][rx+1] == 0) {
        maze[ry][rx] = 1;  // Add a wall
      }
    }
  }
}

// Draw the entire maze
void drawMaze() {
  // Clear the screen below the title area
  ucg.setColor(COLOR_BLACK);
  ucg.drawBox(0, 20, SCREEN_WIDTH, SCREEN_HEIGHT - 20);
  
  // Draw the maze
  for (int row = 0; row < MAZE_HEIGHT; row++) {
    for (int col = 0; col < MAZE_WIDTH; col++) {
      int x = col * BLOCK_SIZE;
      int y = row * BLOCK_SIZE + 20; // Offset for title
      
      if (maze[row][col] == 1) {
        drawBlock(x, y, COLOR_RED);
      } else if (maze[row][col] == 2) {
        goalX = x;
        goalY = y;
        drawBlock(x, y, COLOR_YELLOW);
      }
    }
  }
  
  // Draw player
  drawBlock(playerX, playerY + 20, COLOR_GREEN); // +20 for title offset
}

// Handle joystick input with fixed X-axis handling
void handleJoystick() {
  if (millis() - lastMove < moveDelay) return;
  
  int xVal = analogRead(JOY_X);
  int yVal = analogRead(JOY_Y);
  
  // Debug joystick values
  // Serial.print("X: "); Serial.print(xVal);
  // Serial.print(" Y: "); Serial.println(yVal);
  
  int newX = playerX;
  int newY = playerY;
  bool moved = false;
  
  // Process X movement first (prioritize horizontal movement)
  if (xVal < 400) {
    newX -= BLOCK_SIZE;
    moved = true;
  } 
  else if (xVal > 600) {
    newX += BLOCK_SIZE;
    moved = true;
  }
  
  // If no X movement, then check Y movement
  if (!moved) {
    if (yVal < 400) {
      newY -= BLOCK_SIZE;
      moved = true;
    } 
    else if (yVal > 600) {
      newY += BLOCK_SIZE;
      moved = true;
    }
  }
  
  // If no movement detected, return
  if (!moved) return;
  
  // Update timestamp for movement
  lastMove = millis();
  
  int gridX = newX / BLOCK_SIZE;
  int gridY = newY / BLOCK_SIZE;
  
  // Check if the new position is valid
  if (gridY >= 0 && gridY < MAZE_HEIGHT && gridX >= 0 && gridX < MAZE_WIDTH && maze[gridY][gridX] != 1) {
    // Erase player at old position
    drawBlock(playerX, playerY + 20, COLOR_BLACK); // +20 for title offset
    
    // Update player position
    playerX = newX;
    playerY = newY;
    
    // Draw player at new position
    drawBlock(playerX, playerY + 20, COLOR_GREEN); // +20 for title offset
    
    // Check if player reached the goal
    if (maze[gridY][gridX] == 2) {
      gameWon = true;
    }
  }
}

// Check for joystick button press with improved debounce
void checkJoystickButton() {
  // Only check button every few milliseconds to avoid bouncing
  if (millis() - lastButtonCheck < 50) return;
  lastButtonCheck = millis();
  
  // Read button state - LOW means pressed (pulled to ground)
  int buttonState = digitalRead(JOY_BTN);
  
  // Debug button state
  // Serial.print("Button: "); Serial.println(buttonState);
  
  // Check if button is pressed (active LOW)
  if (buttonState == LOW) {
    if (!buttonPressed) {
      // Button was just pressed
      buttonPressed = true;
      
      // Add debug tone to confirm button works
      tone(BUZZER_PIN, 1000, 100);
      
      // Button action goes here (depends on game state)
      if (gameWon) {
        // If in win screen, this will be handled by showWinScreen()
      } else {
        // Regular game - could add pause menu or other action
        Serial.println("Button pressed during gameplay");
      }
    }
  } else {
    // Button is released
    buttonPressed = false;
  }
}

// Display win screen with improved button detection
void showWinScreen() {
  // Play victory sound
  tone(BUZZER_PIN, 500, 300);
  delay(300);
  tone(BUZZER_PIN, 700, 300);
  delay(300);
  tone(BUZZER_PIN, 900, 300);
  delay(300);
  noTone(BUZZER_PIN);
  
  // Draw victory message
  int boxW = 200;
  int boxH = 80;
  int boxX = (SCREEN_WIDTH - boxW) / 2;
  int boxY = (SCREEN_HEIGHT - boxH) / 2;
  
  ucg.setColor(COLOR_MAGENTA);
  ucg.drawBox(boxX, boxY, boxW, boxH);
  ucg.setColor(COLOR_WHITE);
  ucg.drawFrame(boxX, boxY, boxW, boxH);
  
  ucg.setColor(COLOR_WHITE);
  ucg.setFont(ucg_font_ncenR14_hr);
  ucg.setPrintPos(boxX + 25, boxY + 30);
  ucg.print("Maze Complete!");
  ucg.setPrintPos(boxX + 15, boxY + 55);
  ucg.print("Press joystick");
  
  // Reset button state
  buttonPressed = false;
  
  // Better button detection loop
  unsigned long startWaitTime = millis();
  bool buttonWasPressed = false;
  
  while (!buttonWasPressed) {
    // Flash the message to indicate waiting for input
    if ((millis() / 500) % 2 == 0) {
      ucg.setColor(COLOR_WHITE);
    } else {
      ucg.setColor(COLOR_YELLOW);
    }
    ucg.setPrintPos(boxX + 15, boxY + 55);
    ucg.print("Press joystick");
    
    // Check button with debounce
    int buttonState = digitalRead(JOY_BTN);
    if (buttonState == LOW) {
      // Button is pressed (active LOW)
      if (!buttonPressed) {
        buttonPressed = true;
        // Play confirmation sound
        tone(BUZZER_PIN, 1000, 100);
        delay(200); // Wait a bit to avoid immediate next-level generation
        buttonWasPressed = true;
      }
    } else {
      buttonPressed = false;
    }
    
    // Add a timeout or alternate method (like any joystick movement)
    // in case button is completely non-functional
    if (millis() - startWaitTime > 10000) { // 10 second timeout
      buttonWasPressed = true; // Force continuation
    }
    
    // Also check for significant joystick movement as alternative
    int xVal = analogRead(JOY_X);
    int yVal = analogRead(JOY_Y);
    if (xVal < 200 || xVal > 800 || yVal < 200 || yVal > 800) {
      // Extreme joystick position can also advance
      delay(500); // Debounce
      buttonWasPressed = true;
    }
    
    delay(50); // Small delay for responsiveness
  }
  
  // Advance to next level
  currentLevel++;
  resetGame();
}

// Reset the game
void resetGame() {
  generateMaze();
  drawTitle();
  drawMaze();
  gameWon = false;
}

void setup() {
  // Initialize hardware
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);
  pinMode(JOY_BTN, INPUT_PULLUP); // Using internal pullup
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Add serial for debugging
  Serial.begin(9600);
  Serial.println("Maze Game Starting");
  
  // Test button functionality at startup
  Serial.println("Testing button - press joystick button...");
  unsigned long buttonTestStart = millis();
  bool buttonWorking = false;
  
  // Quick button test at startup
  while (millis() - buttonTestStart < 3000) {
    if (digitalRead(JOY_BTN) == LOW) {
      tone(BUZZER_PIN, 1000, 100);
      Serial.println("Button works!");
      buttonWorking = true;
      break;
    }
    delay(50);
  }
  
  if (!buttonWorking) {
    Serial.println("No button press detected in first 3 seconds");
    Serial.println("Game will continue anyway");
  }
  
  // Initialize random seed
  randomSeed(analogRead(A2));
  
  // Initialize display
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  // Set to landscape mode - using 90 degree rotation as ILI9341 is naturally portrait
  ucg.setRotate90();
  ucg.clearScreen();
  
  // Initialize game
  currentLevel = 1;
  resetGame();
}

void loop() {
  // Check for button press regardless of game state
  checkJoystickButton();
  
  if (!gameWon) {
    handleJoystick();
  } else {
    showWinScreen();
  }
}

#endif