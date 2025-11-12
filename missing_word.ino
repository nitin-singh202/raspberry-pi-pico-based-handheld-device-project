// A means up , B means bottom , # means select 
#include <Arduino.h>
#include <Ucglib.h>  
#include <Keypad.h>
#include <SPI.h>

// TFT Pins (your specified pins)
#define TFT_CS   17
#define TFT_DC   15
#define TFT_RST  16

// Change BUZZER_PIN to avoid conflict with keypad
#define BUZZER_PIN 5  
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Create Ucglib TFT instance
Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

// Colors for Ucglib - RGB format
#define BLACK    0, 0, 0
#define WHITE    255, 255, 255
#define CYAN     0, 255, 255
#define YELLOW   255, 255, 0
#define GREEN    0, 255, 0
#define DARKGREY 64, 64, 64
#define BLUE     0, 0, 255
#define RED      255, 0, 0

// Keypad configuration
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Keypad pins - changed to byte type instead of int
byte rowPins[4] = {13, 12, 11, 10};
byte colPins[4] = {9, 8, 7, 6};  
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Game Data
String objects[] = {"Cat", "Apple", "Ball", "Star", "Tree", "Cup", "Moon", "Book", 
                    "Car", "House", "Fish", "Bird", "Pen", "Key", "Hat", "Sun"};
int currentSet[6];
int numObjects = 4;
int score = 0;
int missingIndex = 0;
int selectedOption = 0;  // Now represents position in 2x2 grid (0-3)
int gameLevel = 1;

// Layout Constants
const int HEADER_HEIGHT = 30;
const int INSTRUCTIONS_HEIGHT = 15;
const int OPTIONS_AREA_Y = 170;  // Adjusted for 2x2 layout
const int OPTION_WIDTH = 100;    // Width for each option box
const int OPTION_HEIGHT = 20;    // Height for each option box
const int OPTION_SPACING = 7;   // Space between option boxes
const int GRID_START_Y = HEADER_HEIGHT + 10;
const int GRID_CELL_WIDTH = 90;
const int GRID_CELL_HEIGHT = 40;
const int GRID_SPACING = 10;

// Forward declarations
void drawGameScreen();
void showOverlay(const char* msg, const char* btn);
void playCorrectTone();
void playWrongTone();
void drawOptions();
void handleSelection();

void playCorrectTone() {
  tone(BUZZER_PIN, 1000, 150);
  delay(200);
  noTone(BUZZER_PIN);
}

void playWrongTone() {
  tone(BUZZER_PIN, 400, 300);
  delay(300);
  noTone(BUZZER_PIN);
}

void showOverlay(const char* msg, const char* btn) {
  int overlayW = 200;
  int overlayH = 80;
  int x = (SCREEN_WIDTH - overlayW) / 2;
  int y = (SCREEN_HEIGHT - overlayH) / 2;
  
  ucg.setColor(DARKGREY);
  ucg.drawBox(x, y, overlayW, overlayH);
  ucg.setColor(WHITE);
  ucg.drawFrame(x, y, overlayW, overlayH);
  
  ucg.setFont(ucg_font_helvR14_tr);
  ucg.setColor(WHITE);
  ucg.setPrintPos(x + (overlayW - strlen(msg) * 8) / 2, y + 35);
  ucg.print(msg);
  
  ucg.setColor(YELLOW);
  ucg.setPrintPos(x + (overlayW - (strlen(btn) + 3) * 8) / 2, y + 60);
  ucg.print(">> ");
  ucg.print(btn);
}

void drawOptions() {
  // Clear options area
  ucg.setColor(BLACK);
  ucg.drawBox(0, OPTIONS_AREA_Y - INSTRUCTIONS_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - OPTIONS_AREA_Y + INSTRUCTIONS_HEIGHT);
  
  // Draw separator line
  ucg.setColor(WHITE);
  ucg.drawLine(0, OPTIONS_AREA_Y - INSTRUCTIONS_HEIGHT - 5, SCREEN_WIDTH, OPTIONS_AREA_Y - INSTRUCTIONS_HEIGHT - 5);
  
  // Display control instructions
  ucg.setFont(ucg_font_helvR10_tr);
  ucg.setColor(WHITE);
  ucg.setPrintPos(10, OPTIONS_AREA_Y - 5);
  ucg.print("A:Up  B:Down  C:Left  D:Right  #:Select");
  
  // Calculate starting position for 2x2 grid
  int startX = (SCREEN_WIDTH - (2 * OPTION_WIDTH + OPTION_SPACING)) / 2;
  
  // Display options in 2x2 grid
  ucg.setFont(ucg_font_helvR12_tr);
  for (int i = 0; i < numObjects; i++) {
    int row = i / 2;
    int col = i % 2;
    int x = startX + col * (OPTION_WIDTH + OPTION_SPACING);
    int y = OPTIONS_AREA_Y + 15 + row * (OPTION_HEIGHT + OPTION_SPACING);
    
    // Draw option box background
    if (i == selectedOption) {
      ucg.setColor(GREEN);
      ucg.drawBox(x, y, OPTION_WIDTH, OPTION_HEIGHT);
      ucg.setColor(BLACK);
    } else {
      ucg.setColor(WHITE);
      ucg.drawFrame(x, y, OPTION_WIDTH, OPTION_HEIGHT);
    }
    
    // Draw option text centered in box
    ucg.setColor(i == selectedOption ? BLACK : WHITE);
    int textWidth = objects[currentSet[i]].length() * 6;
    int textX = x + (OPTION_WIDTH - textWidth) / 2;
    int textY = y + (OPTION_HEIGHT + 8) / 2;
    
    ucg.setPrintPos(textX, textY);
    ucg.print(objects[currentSet[i]]);
  }
}

void drawGameScreen() {
  ucg.setColor(BLACK);
  ucg.drawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  
  // Draw header with score and level
  ucg.setColor(BLUE);
  ucg.drawBox(0, 0, SCREEN_WIDTH, HEADER_HEIGHT);
  
  ucg.setFont(ucg_font_helvR12_tr);
  ucg.setColor(WHITE);
  ucg.setPrintPos(10, 20);
  ucg.print("Score: ");
  ucg.print(score);
  
  ucg.setPrintPos(250, 20);
  ucg.print("Lvl: ");
  ucg.print(gameLevel);
  
  // Calculate grid layout
  int gridStartX;
  int cols;
  int rows;
  
  if (numObjects <= 4) {
    cols = 2;
    rows = 2;
  } else {
    cols = 3;
    rows = 2;
  }
  
  gridStartX = (SCREEN_WIDTH - (cols * GRID_CELL_WIDTH + (cols - 1) * GRID_SPACING)) / 2;
  
  // Randomly select objects
  for (int i = 0; i < numObjects; i++) {
    bool unique;
    do {
      currentSet[i] = random(0, sizeof(objects)/sizeof(objects[0]));
      unique = true;
      for (int j = 0; j < i; j++) {
        if (currentSet[j] == currentSet[i]) {
          unique = false;
          break;
        }
      }
    } while (!unique);
  }
  
  // Draw objects as text placeholders in a grid
  for (int i = 0; i < numObjects; i++) {
    int col = i % cols;
    int row = i / cols;
    int x = gridStartX + col * (GRID_CELL_WIDTH + GRID_SPACING);
    int y = GRID_START_Y + row * (GRID_CELL_HEIGHT + GRID_SPACING);
    
    // Draw frame around object
    ucg.setColor(WHITE);
    ucg.drawFrame(x, y, GRID_CELL_WIDTH, GRID_CELL_HEIGHT);
    
    // Center text in box
    ucg.setFont(ucg_font_helvR10_tr);
    int textWidth = objects[currentSet[i]].length() * 6;
    int textX = x + (GRID_CELL_WIDTH - textWidth) / 2;
    int textY = y + (GRID_CELL_HEIGHT + 8) / 2;  // Adjust for font height
    
    ucg.setColor(CYAN);
    ucg.setPrintPos(textX, textY);
    ucg.print(objects[currentSet[i]]);
  }
  
  // Show objects for a few seconds (time decreases with level)
  int viewTime = max(1000, 3000 - (gameLevel - 1) * 300);
  delay(viewTime);
  
  // Randomly remove one
  missingIndex = random(0, numObjects);
  int col = missingIndex % cols;
  int row = missingIndex / cols;
  int x = gridStartX + col * (GRID_CELL_WIDTH + GRID_SPACING);
  int y = GRID_START_Y + row * (GRID_CELL_HEIGHT + GRID_SPACING);
  
  // Erase the missing object
  ucg.setColor(BLACK);
  ucg.drawBox(x + 1, y + 1, GRID_CELL_WIDTH - 2, GRID_CELL_HEIGHT - 2);
  ucg.setColor(RED);
  ucg.drawFrame(x, y, GRID_CELL_WIDTH, GRID_CELL_HEIGHT);
  
  delay(500);
  drawOptions();
}

void handleSelection() {
  char key = keypad.getKey();
  
  if (key) {
    // Navigation in 2x2 grid
    if (key == 'A' && selectedOption >= 2) {  // Up
      selectedOption -= 2;
      drawOptions();
    }
    
    if (key == 'B' && selectedOption < numObjects - 2) {  // Down
      selectedOption += 2;
      drawOptions();
    }
    
    if (key == 'C' && selectedOption % 2 != 0) {  // Left
      selectedOption--;
      drawOptions();
    }
    
    if (key == 'D' && selectedOption % 2 == 0 && selectedOption < numObjects - 1) {  // Right
      selectedOption++;
      drawOptions();
    }
    
    // Selection
    if (key == '#') {
      if (selectedOption == missingIndex) {
        // Correct answer
        score++;
        playCorrectTone();
        showOverlay("Correct!", "Next");
        delay(1500);
        
        // Increase difficulty every 3 points
        if (score % 3 == 0 && numObjects < 6) {
          numObjects++;
          gameLevel++;
        }
      } else {
        // Incorrect answer
        playWrongTone();
        showOverlay("Incorrect", "Retry");
        delay(1500);
      }
      
      // Reset selection and start new round
      selectedOption = 0;
      drawGameScreen();
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Initialize random seed using a different analog pin or micros()
  randomSeed(micros());  
  
  // Initialize display
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.setRotate90(); // Landscape mode
  
  // Show welcome screen
  ucg.setColor(BLACK);
  ucg.drawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  
  ucg.setFont(ucg_font_helvR18_tr);
  ucg.setColor(WHITE);
  ucg.setPrintPos(30, SCREEN_HEIGHT/2 - 20);
  ucg.print("What's Missing?");
  
  ucg.setFont(ucg_font_helvR12_tr);
  ucg.setPrintPos(50, SCREEN_HEIGHT/2 + 40);
  ucg.print("Press any key to start");
  
  // Display splash screen for longer - INCREASED DURATION
  delay(3000);  // This adds a 3-second delay before waiting for key press
  
  // Optional: Add a simple animation or blinking text during splash screen
  for (int i = 0; i < 4; i++) {
    // Blink the "Press any key to start" text
    ucg.setColor(BLACK);
    ucg.drawBox(50, SCREEN_HEIGHT/2 + 28, 200, 20);
    delay(250);
    
    ucg.setColor(WHITE);
    ucg.setPrintPos(50, SCREEN_HEIGHT/2 + 40);
    ucg.print("Press any key to start");
    delay(250);
  }
  
  // Wait for a key press to start
  while (!keypad.getKey()) {
    delay(50);
  }
  
  // Start the game
  drawGameScreen();
}

void loop() {
  handleSelection();
  delay(50); // Small delay for stability
}