#include <Arduino.h>
#include <SPI.h>
#include <Ucglib.h>

// Define TFT pins - adjust according to your connections
#define TFT_CS   17
#define TFT_DC   15
#define TFT_RST  16
#define JOYSTICK_BUTTON_PIN 20

//#define BUZZER_PIN 9

// Initialize UCGLIB display in landscape mode
// Using hardware SPI. Adjust constructor parameters according to your setup
// Parameters: dc, cs, reset
Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

// Colors
#define COLOR_BLACK     0, 0, 0
#define COLOR_WHITE     255, 255, 255
#define COLOR_RED       255, 0, 0
#define COLOR_GREEN     0, 255, 0
#define COLOR_BLUE      0, 0, 255
#define COLOR_YELLOW    255, 255, 0
#define COLOR_DARKGREY  64, 64, 64

// Screen Dimensions (in landscape mode)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Paddle
#define PADDLE_WIDTH 60
#define PADDLE_HEIGHT 10
#define PADDLE_Y (SCREEN_HEIGHT - 20)
int paddleX = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;

// Ball
int ballX, ballY;
int ballRadius = 5;
float ballVX, ballVY;

// Game
int score = 0;
bool gameRunning = false;
float speedMultiplier = 2.5;

// Joystick or potentiometer pins for paddle control
#define JOYSTICK_X_PIN A1

// Function prototypes
void resetGame();
void drawGame();
void updateBall();
void updatePaddle(int dx);
void showGameOver();
void startPingPongGame();
void drawCircle(int x, int y, int radius, uint8_t r, uint8_t g, uint8_t b);
void fillCircle(int x, int y, int radius, uint8_t r, uint8_t g, uint8_t b);

void setup() {
  // Initialize serial for debugging
  Serial.begin(9600);
  pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLUP);

  // Initialize the display
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.clearScreen();
  
  // Set rotation for landscape mode
  ucg.setRotate90();
  
  // Initialize buzzer
 // pinMode(BUZZER_PIN, OUTPUT);
  
  // Set random seed
  randomSeed(analogRead(A1));
  
  // Start the game
  startPingPongGame();
}

void loop() {
  if (gameRunning) {
    // Read paddle control (joystick or potentiometer)
    int joystickValue = analogRead(JOYSTICK_X_PIN);
    int paddleSpeed = map(joystickValue, 0, 1023, -10, 10);
    
    // Only update paddle if there's significant movement
    if (abs(paddleSpeed) > 1) {
      updatePaddle(paddleSpeed);
    }
    
    // Update ball position and check collisions
    updateBall();
    
    // Small delay for game speed
    delay(20);
  } else {
    // Check for button press to restart game
    // For demo, restart after delay
    // Check for serial input to restart (optional)
    if (Serial.available() > 0) {
      char input = Serial.read();
      if (input == 'r') {
        resetGame();
      }
    }

    // Check if joystick button is pressed to restart
    if (digitalRead(JOYSTICK_BUTTON_PIN) == LOW) {
      delay(200); // debounce delay
      resetGame();
    }
  }
}

// Helper functions for drawing circles in UCGLIB
void drawCircle(int x, int y, int radius, uint8_t r, uint8_t g, uint8_t b) {
  ucg.setColor(r, g, b);
  for (int i = 0; i < 360; i += 5) {
    float angle = i * DEG_TO_RAD;
    int xPos = x + cos(angle) * radius;
    int yPos = y + sin(angle) * radius;
    ucg.drawPixel(xPos, yPos);
  }
}

void fillCircle(int x, int y, int radius, uint8_t r, uint8_t g, uint8_t b) {
  ucg.setColor(r, g, b);
  for (int i = 0; i <= radius; i++) {
    drawCircle(x, y, i, r, g, b);
  }
}

void resetGame() {
  paddleX = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
  ballX = random(20, SCREEN_WIDTH - 20);
  ballY = 30;
  ballVX = 2;
  ballVY = 2;
  score = 0;
  speedMultiplier = 2.5;
  gameRunning = true;
  drawGame();
}

void drawGame() {
  // Clear screen
  ucg.setColor(COLOR_BLACK);
  ucg.clearScreen();

  // Draw paddle
  ucg.setColor(COLOR_BLUE);
  ucg.drawBox(paddleX, PADDLE_Y, PADDLE_WIDTH, PADDLE_HEIGHT);

  // Draw ball
  fillCircle(ballX, ballY, ballRadius, COLOR_RED);

  // Draw score
  ucg.setColor(COLOR_WHITE);
  ucg.setFont(ucg_font_ncenR14_hr);
  ucg.setPrintPos(SCREEN_WIDTH - 100, 20);
  ucg.print("Score: ");
  ucg.print(score);
}

void updateBall() {
  // Clear old ball
  fillCircle(ballX, ballY, ballRadius, COLOR_BLACK);

  // Update ball position
  ballX += ballVX * speedMultiplier;
  ballY += ballVY * speedMultiplier;

  // Collisions with walls
  if (ballX - ballRadius <= 0 || ballX + ballRadius >= SCREEN_WIDTH) {
    ballVX = -ballVX;
    // Add slight y-speed randomization for more interesting bounces
    ballVY += random(-1, 2) * 0.1;
  }
  
  if (ballY - ballRadius <= 0) {
    ballVY = -ballVY;
  }

  // Collision with paddle
  if (ballY + ballRadius >= PADDLE_Y && ballY + ballRadius <= PADDLE_Y + PADDLE_HEIGHT) {
    if (ballX >= paddleX && ballX <= paddleX + PADDLE_WIDTH) {
      ballVY = -ballVY;
      
      // Angle the ball based on where it hit the paddle
      float hitPosition = (ballX - paddleX) / PADDLE_WIDTH;
      ballVX = (hitPosition - 0.5) * 6; // -3 to +3 based on hit position
      
      score++;
      //tone(BUZZER_PIN, 1500, 100);

      // Increase speed every 5 points
      if (score % 5 == 0 && speedMultiplier < 2.0) {
        speedMultiplier += 0.4;
      }
    }
  }

  // Missed paddle
  if (ballY - ballRadius > SCREEN_HEIGHT) {
    gameRunning = false;
    showGameOver();
    return;
  }

  // Draw new ball
  fillCircle(ballX, ballY, ballRadius, COLOR_RED);

  // Update score
  ucg.setColor(COLOR_BLACK);
  ucg.drawBox(SCREEN_WIDTH - 100, 5, 95, 20);
  ucg.setColor(COLOR_WHITE);
  ucg.setFont(ucg_font_ncenR14_hr);
  ucg.setPrintPos(SCREEN_WIDTH - 100, 20);
  ucg.print("Score: ");
  ucg.print(score);
}

void updatePaddle(int dx) {
  // Clear old paddle
  ucg.setColor(COLOR_BLACK);
  ucg.drawBox(paddleX, PADDLE_Y, PADDLE_WIDTH, PADDLE_HEIGHT);
  
  // Update paddle position
  paddleX += dx;
  paddleX = constrain(paddleX, 0, SCREEN_WIDTH - PADDLE_WIDTH);
  
  // Draw new paddle
  ucg.setColor(COLOR_BLUE);
  ucg.drawBox(paddleX, PADDLE_Y, PADDLE_WIDTH, PADDLE_HEIGHT);
}

void showGameOver() {
  // Game over sound
  //tone(BUZZER_PIN, 300, 200);
  //delay(250);
  //tone(BUZZER_PIN, 200, 200);
  // delay(250);
  // tone(BUZZER_PIN, 100, 300);

  // Game over overlay
  int overlayW = SCREEN_WIDTH / 2;
  int overlayH = SCREEN_HEIGHT / 2;
  int x = (SCREEN_WIDTH - overlayW) / 2;
  int y = (SCREEN_HEIGHT - overlayH) / 2;

  ucg.setColor(COLOR_DARKGREY);
  ucg.drawBox(x, y, overlayW, overlayH);
  
  ucg.setColor(COLOR_WHITE);
  ucg.setFont(ucg_font_ncenR24_hr);
  ucg.setPrintPos(x + 20, y + 40);
  ucg.print("Game Over");

  ucg.setFont(ucg_font_ncenR14_hr);
  ucg.setPrintPos(x + 20, y + 70);
  ucg.print("Score: ");
  ucg.print(score);

  ucg.setColor(COLOR_YELLOW);
  ucg.setPrintPos(x + 30, y + 100);
  ucg.print(">> Press joystick to retry");  // Changed from "Press 'r' to Retry"
}

void startPingPongGame() {
  // Start screen
  ucg.setColor(COLOR_BLACK);
  ucg.clearScreen();
  
  ucg.setColor(COLOR_WHITE);
  ucg.setFont(ucg_font_ncenR24_hr);
  
  // Center the title
  const char* title = "Ping Pong";
  int titleWidth = strlen(title) * 15;  // Approximate width
  int titleX = (SCREEN_WIDTH - titleWidth) / 2;
  
  ucg.setPrintPos(titleX, SCREEN_HEIGHT / 2 - 20);
  ucg.print(title);
  
  // Display instructions
  ucg.setFont(ucg_font_ncenR14_hr);
  ucg.setPrintPos(50, SCREEN_HEIGHT / 2 + 20);
  ucg.print("Use joystick to move paddle");
  
  delay(2000);
  resetGame();
}