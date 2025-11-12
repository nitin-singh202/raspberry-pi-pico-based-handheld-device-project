#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

#include <Ucglib.h>

// Define your display pins here
#define TFT_CS 17    // CS pin
#define TFT_DC 15    // DC/RS pin 
#define TFT_RST 16    // Reset pin (or connect to Arduino reset)

// Display object - update these pins to match your wiring
// This uses hardware SPI for better performance
Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

#define JOY_X A0
#define JOY_Y A1
#define JOY_BTN 4
#define BUZZER_PIN 5

// Display is in landscape mode (320×240)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BLOCK_SIZE 10

#define MAX_LENGTH 100

// Play area margins (visible wall area)
#define WALL_THICKNESS 4
#define PLAY_X1 WALL_THICKNESS
#define PLAY_Y1 WALL_THICKNESS
#define PLAY_X2 (SCREEN_WIDTH - WALL_THICKNESS)
#define PLAY_Y2 (SCREEN_HEIGHT - WALL_THICKNESS)

// Color definitions
#define COLOR_BLACK 0, 0, 0
#define COLOR_GREEN 0, 255, 0
#define COLOR_RED 255, 0, 0  
#define COLOR_MAGENTA 255, 0, 255
#define COLOR_YELLOW 255, 255, 0
#define COLOR_WHITE 255, 255, 255

bool isLandscape = true;  // Start in landscape mode

int snakeX[MAX_LENGTH], snakeY[MAX_LENGTH];
int length = 5;
int dirX = 1, dirY = 0;

int fruitX, fruitY;
int score = 0;
bool gameOver = false;

unsigned long lastMove = 0;
const int moveDelay = 150;

void spawnFruit() {
  fruitX = random(PLAY_X1 / BLOCK_SIZE + 1, PLAY_X2 / BLOCK_SIZE - 1) * BLOCK_SIZE;
  fruitY = random(PLAY_Y1 / BLOCK_SIZE + 1, PLAY_Y2 / BLOCK_SIZE - 1) * BLOCK_SIZE;
}

void drawBlock(int x, int y, const uint8_t r, const uint8_t g, const uint8_t b) {
  ucg.setColor(r, g, b);
  ucg.drawBox(x, y, BLOCK_SIZE, BLOCK_SIZE);
}

void drawSnake() {
  for (int i = 0; i < length; i++) {
    drawBlock(snakeX[i], snakeY[i], COLOR_GREEN);
  }
}

void drawWalls() {
  ucg.setColor(COLOR_RED);
  ucg.drawFrame(PLAY_X1, PLAY_Y1, PLAY_X2 - PLAY_X1, PLAY_Y2 - PLAY_Y1);
}

void eraseTail() {
  drawBlock(snakeX[length - 1], snakeY[length - 1], COLOR_BLACK);
}

void drawScore() {
  // Clear previous score area
  ucg.setColor(COLOR_BLACK);
  ucg.drawBox(SCREEN_WIDTH - 80, 0, 80, 20);

  // Draw updated score
  ucg.setColor(COLOR_YELLOW);
  ucg.setFont(ucg_font_ncenR12_tr);  // Slightly smaller font for better fit
  ucg.setPrintPos(SCREEN_WIDTH - 75, 15);
  ucg.print("Score:");
  ucg.print(score);
}

void handleJoystick() {
  int xVal = analogRead(JOY_X);
  int yVal = analogRead(JOY_Y);

  if (xVal < 400 && dirX == 0) { dirX = -1; dirY = 0; }
  else if (xVal > 600 && dirX == 0) { dirX = 1; dirY = 0; }
  else if (yVal < 400 && dirY == 0) { dirX = 0; dirY = -1; }
  else if (yVal > 600 && dirY == 0) { dirX = 0; dirY = 1; }
}

void resetGame() {
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);

  // Set to default orientation based on the isLandscape flag
  if (isLandscape) {
    ucg.setRotate90(); // 90° is landscape on ILI9341
  } else {
    ucg.setRotate270(); // 270° = portrait flipped
  }

  ucg.clearScreen();
  drawWalls();
  length = 5;
  score = 0;
  dirX = 1; dirY = 0;

  for (int i = 0; i < length; i++) {
    snakeX[i] = 60 - i * BLOCK_SIZE;
    snakeY[i] = 60;
  }

  spawnFruit();
  drawScore();  // Show initial score
  gameOver = false;
  drawSnake();
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

void showGameOverScreen() {
  playGameOverSound();

  int boxW = SCREEN_WIDTH / 2;
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
  ucg.print("Score: ");
  ucg.print(score);

  ucg.setPrintPos(boxX + 10, boxY + 75);
  ucg.print("Press joystick to retry");

  // Wait for joystick button press
  bool buttonPressed = false;
  while (!buttonPressed) {
    if (digitalRead(JOY_BTN) == LOW) {
      delay(200);  // Debounce
      buttonPressed = true;

      resetGame();
      gameOver = false;
    }
  }
}


void updateSnake() {
  if (millis() - lastMove < moveDelay) return;
  lastMove = millis();

  eraseTail();

  for (int i = length - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  snakeX[0] += dirX * BLOCK_SIZE;
  snakeY[0] += dirY * BLOCK_SIZE;

  // Wall collision
  if (snakeX[0] < PLAY_X1 || snakeY[0] < PLAY_Y1 || 
      snakeX[0] >= PLAY_X2 || snakeY[0] >= PLAY_Y2) {
    gameOver = true;
    return;
  }

  // Self collision
  for (int i = 1; i < length; i++) {
    if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
      gameOver = true;
      return;
    }
  }

  // Fruit collision
  if (snakeX[0] == fruitX && snakeY[0] == fruitY) {
    if (length < MAX_LENGTH) length++;
    score++;
    drawScore();  // Update score display
    spawnFruit();
  }

  drawBlock(snakeX[0], snakeY[0], COLOR_GREEN);
  drawBlock(fruitX, fruitY, COLOR_MAGENTA);
}

void setup() {
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);
  pinMode(JOY_BTN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Initialize random seed
  randomSeed(analogRead(A2));  // Using an unconnected analog pin for random seed
  
  // Initialize display
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  // Set to landscape mode - using 90 degree rotation as ILI9341 is naturally portrait
  ucg.setRotate90();  // No toggle or change in orientation
  ucg.clearScreen();
  
  resetGame();
}

void loop() {
  handleJoystick();
  updateSnake();

  if (gameOver) {
    showGameOverScreen();
  }

  // Check if joystick button is pressed (restart + rotate)
  if (digitalRead(JOY_BTN) == LOW) {
    delay(200);  // Debounce
    resetGame();  // Restart game
  }
}

#endif