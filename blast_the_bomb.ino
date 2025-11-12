#ifndef BLAST_THE_BOMB_H
#define BLAST_THE_BOMB_H

#include <Ucglib.h>

// Display configuration (matching the maze game)
#define TFT_CS   17
#define TFT_DC   15
#define TFT_RST  16
#define BUZZER_PIN 0

// Joystick configuration (matching the maze game)
#define JOY_X A0
#define JOY_Y A1
#define JOY_BTN 4

// Create display instance with hardware SPI (matching the maze game)
Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

// Screen parameters
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Game object dimensions
#define PLAYER_WIDTH 30
#define PLAYER_HEIGHT 10
#define BULLET_RADIUS 3
#define BOMB_RADIUS 6

// Color definitions
#define COLOR_BLACK 0, 0, 0
#define COLOR_WHITE 255, 255, 255
#define COLOR_RED 255, 0, 0
#define COLOR_GREEN 0, 255, 0
#define COLOR_BLUE 0, 0, 255
#define COLOR_YELLOW 255, 255, 0
#define COLOR_DARKGREY 50, 50, 50

int playerX = (SCREEN_WIDTH - PLAYER_WIDTH) / 2;

#define MAX_BULLETS 5
#define MAX_BOMBS 10

struct Bullet {
  int x, y;
  bool active;
};

struct Bomb {
  int x, y;
  bool active;
};

Bullet bullets[MAX_BULLETS];
Bomb bombs[MAX_BOMBS];

int score = 0;
bool gameRunning = false;
unsigned long lastBombTime = 0;
unsigned long bombInterval = 1000;
float bombSpeed = 1.5;

// Function prototypes
void drawPlayer();
void erasePlayer();
void drawBullet(Bullet &b);
void eraseBullet(Bullet &b);
void drawBomb(Bomb &b);
void eraseBomb(Bomb &b);
void resetGame();
void updateScore();
void fireBullet();
void spawnBomb();
void updateBullets();
void updateBombs();
void updatePlayer(int dx);
void showGameOver();
void startBlastTheBombGame();

void drawPlayer() {
  ucg.setColor(COLOR_BLUE);
  ucg.drawBox(playerX, SCREEN_HEIGHT - PLAYER_HEIGHT - 5, PLAYER_WIDTH, PLAYER_HEIGHT);
}

void erasePlayer() {
  ucg.setColor(COLOR_BLACK);
  ucg.drawBox(playerX, SCREEN_HEIGHT - PLAYER_HEIGHT - 5, PLAYER_WIDTH, PLAYER_HEIGHT);
}

void drawBullet(Bullet &b) {
  ucg.setColor(COLOR_GREEN);
  // Draw bullet as a small box (since ucg doesn't have direct circle drawing)
  ucg.drawBox(b.x - BULLET_RADIUS, b.y - BULLET_RADIUS, BULLET_RADIUS * 2, BULLET_RADIUS * 2);
}

void eraseBullet(Bullet &b) {
  ucg.setColor(COLOR_BLACK);
  ucg.drawBox(b.x - BULLET_RADIUS, b.y - BULLET_RADIUS, BULLET_RADIUS * 2, BULLET_RADIUS * 2);
}

void drawBomb(Bomb &b) {
  ucg.setColor(COLOR_RED);
  ucg.drawBox(b.x - BOMB_RADIUS, b.y - BOMB_RADIUS, BOMB_RADIUS * 2, BOMB_RADIUS * 2);
}

void eraseBomb(Bomb &b) {
  ucg.setColor(COLOR_BLACK);
  ucg.drawBox(b.x - BOMB_RADIUS, b.y - BOMB_RADIUS, BOMB_RADIUS * 2, BOMB_RADIUS * 2);
}

void resetGame() {
  playerX = (SCREEN_WIDTH - PLAYER_WIDTH) / 2;
  score = 0;
  gameRunning = true;
  bombSpeed = 1.5;
  bombInterval = 1000;
  
  for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
  for (int i = 0; i < MAX_BOMBS; i++) bombs[i].active = false;
  
  ucg.clearScreen();
  drawPlayer();
  updateScore();
}

void updateScore() {
  // Clear score area
  ucg.setColor(COLOR_BLACK);
  ucg.drawBox(SCREEN_WIDTH - 80, 10, 80, 20);
  
  // Draw score text
  ucg.setColor(COLOR_WHITE);
  ucg.setFont(ucg_font_ncenR12_tr);
  ucg.setPrintPos(SCREEN_WIDTH - 70, 25);
  ucg.print("Score: ");
  ucg.print(score);
}

void fireBullet() {
  // Check joystick button press
  if (digitalRead(JOY_BTN) == LOW) {
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (!bullets[i].active) {
        bullets[i].x = playerX + PLAYER_WIDTH / 2;
        bullets[i].y = SCREEN_HEIGHT - PLAYER_HEIGHT - 5;
        bullets[i].active = true;
        
        // Play sound effect for bullet
        tone(BUZZER_PIN, 1000, 50);
        
        break;
      }
    }
  }
}

void spawnBomb() {
  for (int i = 0; i < MAX_BOMBS; i++) {
    if (!bombs[i].active) {
      bombs[i].x = random(BOMB_RADIUS, SCREEN_WIDTH - BOMB_RADIUS);
      bombs[i].y = 0;
      bombs[i].active = true;
      break;
    }
  }
}

void updateBullets() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      eraseBullet(bullets[i]);
      bullets[i].y -= 4;
      if (bullets[i].y < 0) bullets[i].active = false;
      else drawBullet(bullets[i]);
    }
  }
}

void updateBombs() {
  for (int i = 0; i < MAX_BOMBS; i++) {
    if (bombs[i].active) {
      eraseBomb(bombs[i]);
      bombs[i].y += bombSpeed;
      
      // Check for collision with player
      if (bombs[i].y >= SCREEN_HEIGHT - PLAYER_HEIGHT - 5 && 
          bombs[i].x >= playerX && 
          bombs[i].x <= playerX + PLAYER_WIDTH) {
        gameRunning = false;
        showGameOver();
        return;
      }
      
      // Check for collision with bullets
      for (int j = 0; j < MAX_BULLETS; j++) {
        if (bullets[j].active && 
            abs(bombs[i].x - bullets[j].x) < 10 && 
            abs(bombs[i].y - bullets[j].y) < 10) {
          
          bullets[j].active = false;
          eraseBullet(bullets[j]);

          // Explosion effect
          ucg.setColor(COLOR_YELLOW);
          ucg.drawBox(bombs[i].x - (BOMB_RADIUS + 2), bombs[i].y - (BOMB_RADIUS + 2), 
                     (BOMB_RADIUS + 2) * 2, (BOMB_RADIUS + 2) * 2);
          tone(BUZZER_PIN, 800, 100);
          delay(50);
          ucg.setColor(COLOR_BLACK);
          ucg.drawBox(bombs[i].x - (BOMB_RADIUS + 2), bombs[i].y - (BOMB_RADIUS + 2), 
                     (BOMB_RADIUS + 2) * 2, (BOMB_RADIUS + 2) * 2);

          bombs[i].active = false;
          score++;
          updateScore();
          
          // Increase difficulty
          if (score > 10) bombSpeed = 2.5;
          break;
        }
      }
      
      // Remove bomb if it goes off screen
      if (bombs[i].y > SCREEN_HEIGHT) bombs[i].active = false;
      else drawBomb(bombs[i]);
    }
  }
}

void updatePlayer() {
  // Read joystick input
  int xVal = analogRead(JOY_X);
  int dx = 0;
  
  // Map joystick values to movement
  if (xVal < 400) {
    dx = -3; // Move left
  } else if (xVal > 600) {
    dx = 3;  // Move right
  }
  
  if (dx != 0) {
    erasePlayer();
    playerX += dx;
    playerX = constrain(playerX, 0, SCREEN_WIDTH - PLAYER_WIDTH);
    drawPlayer();
  }
}

void showGameOver() {
  // Game over sound
  tone(BUZZER_PIN, 500, 300);
  delay(300);
  tone(BUZZER_PIN, 300, 300);
  delay(300);
  tone(BUZZER_PIN, 100, 500);

  // Display game over screen
  int overlayW = SCREEN_WIDTH / 2;
  int overlayH = SCREEN_HEIGHT / 2;
  int x = (SCREEN_WIDTH - overlayW) / 2;
  int y = (SCREEN_HEIGHT - overlayH) / 2;

  ucg.setColor(COLOR_DARKGREY);
  ucg.drawBox(x, y, overlayW, overlayH);
  
  ucg.setColor(COLOR_WHITE);
  ucg.setFont(ucg_font_ncenR14_hr);
  ucg.setPrintPos(x + 10, y + 40);
  ucg.print("Game Over");

  ucg.setFont(ucg_font_ncenR12_tr);
  ucg.setPrintPos(x + 10, y + 70);
  ucg.print("Score: ");
  ucg.print(score);

  ucg.setColor(COLOR_YELLOW);
  ucg.setPrintPos(x + 30, y + 100);
  ucg.print(">> Press joystick");
  
  // Wait for button press to restart
  bool buttonPressed = false;
  while (!buttonPressed) {
    // Flash text to indicate waiting for input
    if ((millis() / 500) % 2 == 0) {
      ucg.setColor(COLOR_YELLOW);
    } else {
      ucg.setColor(COLOR_WHITE);
    }
    ucg.setPrintPos(x + 30, y + 100);
    ucg.print(">> Press joystick");
    
    // Check joystick button
    if (digitalRead(JOY_BTN) == LOW) {
      tone(BUZZER_PIN, 1000, 100);
      delay(200);
      buttonPressed = true;
    }
    
    delay(50);
  }
  
  resetGame();
}

void startBlastTheBombGame() {
  ucg.clearScreen();
  
  ucg.setColor(COLOR_BLUE);
  ucg.setFont(ucg_font_ncenR14_hr);
  
  // Center the title
  ucg.setPrintPos(40, SCREEN_HEIGHT / 2 - 30);
  ucg.print("Blast the Bomb");
  
  ucg.setFont(ucg_font_ncenR12_tr);
  ucg.setPrintPos(70, SCREEN_HEIGHT / 2 + 20);
  ucg.print("Press joystick to start");
  
  // Wait for button press to start
  while (digitalRead(JOY_BTN) == HIGH) {
    delay(50);
  }
  
  delay(300); // Debounce
  resetGame();
}

void setup() {
  // Initialize serial for debugging
  Serial.begin(9600);
  Serial.println("Blast the Bomb Game Starting");
  
  // Initialize hardware pins
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);
  pinMode(JOY_BTN, INPUT_PULLUP); // Using internal pullup
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Initialize random seed
  randomSeed(analogRead(A2));
  
  // Initialize display
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.setRotate90(); // Set to landscape orientation
  ucg.clearScreen();
  
  // Add startup tone
  tone(BUZZER_PIN, 400, 100);
  delay(100);
  tone(BUZZER_PIN, 600, 100);
  delay(100);
  tone(BUZZER_PIN, 800, 100);
  
  // Start the game
  startBlastTheBombGame();
}

void loop() {
  if (gameRunning) {
    // Process user input using joystick
    updatePlayer();
    
    // Fire bullet when button is pressed
    fireBullet();
    
    // Update game elements
    updateBullets();
    
    // Spawn bombs at intervals
    unsigned long currentTime = millis();
    if (currentTime - lastBombTime > bombInterval) {
      spawnBomb();
      lastBombTime = currentTime;
      
      // Gradually decrease interval (increase difficulty)
      if (bombInterval > 400) {
        bombInterval -= 10;
      }
    }
    
    updateBombs();
  }
  
  // Small delay to control game speed
  delay(30);
}

#endif