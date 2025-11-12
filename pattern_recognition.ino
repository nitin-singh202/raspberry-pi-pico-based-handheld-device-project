#include <Arduino.h>
#include <Ucglib.h>
#include <SPI.h>

// TFT Pins (based on the reference code)
#define TFT_CS   17
#define TFT_DC   15
#define TFT_RST  16
#define BUZZER_PIN 5  // Changed to match reference code

// Display dimensions
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240

// Create Ucglib TFT instance
Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

// Colors for Ucglib - RGB format (matching reference code)
#define BLACK    0, 0, 0
#define WHITE    255, 255, 255
#define RED      255, 0, 0
#define GREEN    0, 255, 0
#define BLUE     0, 0, 255
#define YELLOW   255, 255, 0
#define DARKGREY 64, 64, 64

// Game Variables
int score = 0;
int currentAnswer = 0;
int selectedOption = 0;  // Now represents position in 2x2 grid (0-3)
int difficulty = 1;
String pattern[6]; // Holds the sequence
String options[4]; // Changed to 4 choices for 2x2 grid

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

// Patterns - Using symbols that are more likely to display correctly
String colors[] = {"RED", "YEL", "GRN", "BLU"};
String shapes[] = {"SQR", "CIR", "TRI", "DMD", "REC"};
String stars[]  = {"*", "**", "***", "****", "*****"};  // Fixed star patterns

void playBuzzer(bool correct) {
  tone(BUZZER_PIN, correct ? 1000 : 300, 200);
  delay(200);
  noTone(BUZZER_PIN);
}

void showSplashScreen() {
  ucg.setColor(BLACK);
  ucg.drawBox(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  ucg.setColor(WHITE);
  ucg.setFont(ucg_font_helvR18_tr);
  ucg.setPrintPos(40, 100);
  ucg.print("Pattern Recognition");
  delay(2000);
}

void drawOverlay(const char* message, const char* buttonText) {
  ucg.setColor(DARKGREY);
  ucg.drawBox(60, 80, 200, 100);
  ucg.setColor(WHITE);
  ucg.setFont(ucg_font_helvR14_tr);
  ucg.setPrintPos(80, 120);
  ucg.print(message);
  ucg.setPrintPos(80, 150);
  ucg.setColor(YELLOW);
  ucg.print(">> ");
  ucg.print(buttonText);
}

void drawColorPattern(String text, int x, int y, bool highlight = false) {
  if (text == "RED") {
    ucg.setColor(highlight ? RED : RED);
  } else if (text == "YEL") {
    ucg.setColor(highlight ? YELLOW : YELLOW);
  } else if (text == "GRN") {
    ucg.setColor(highlight ? GREEN : GREEN);
  } else if (text == "BLU") {
    ucg.setColor(highlight ? BLUE : BLUE);
  } else {
    ucg.setColor(highlight ? WHITE : WHITE);
  }
  ucg.setPrintPos(x, y);
  ucg.print(text);
}

void drawPattern() {
  ucg.setColor(BLACK);
  ucg.drawBox(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
  
  // Draw score and difficulty
  ucg.setColor(WHITE);
  ucg.setFont(ucg_font_helvR14_tr);
  ucg.setPrintPos(10, 20);
  ucg.print("Score: ");
  ucg.print(score);
  
  ucg.setPrintPos(DISPLAY_WIDTH - 120, 20);
  ucg.print("Level: ");
  ucg.print(difficulty);
  
  // Draw pattern sequence
  int patternStartX = 30;
  int patternY = 60;
  
  for (int i = 0; i < 5; i++) {
    if (pattern[i].startsWith("RED") || pattern[i].startsWith("YEL") || 
        pattern[i].startsWith("GRN") || pattern[i].startsWith("BLU")) {
      drawColorPattern(pattern[i], patternStartX + i * 50, patternY);
    } else {
      ucg.setColor(WHITE);
      ucg.setPrintPos(patternStartX + i * 50, patternY);
      ucg.print(pattern[i]);
    }
  }
  
  // Draw question mark for the missing pattern
  ucg.setColor(WHITE);
  ucg.setPrintPos(patternStartX + 5 * 50, patternY);
  ucg.print("?");
  
  // Draw instructions
  ucg.setFont(ucg_font_helvR10_tr);
  ucg.setColor(WHITE);
  ucg.setPrintPos(20, OPTIONS_AREA_Y - 20);
  ucg.print("Use joystick to select, press to confirm");
  
  // Draw options in 2x2 grid
  int startX = (DISPLAY_WIDTH - (2 * OPTION_WIDTH + OPTION_SPACING)) / 2;
  
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
    
    if (options[i].startsWith("RED") || options[i].startsWith("YEL") || 
        options[i].startsWith("GRN") || options[i].startsWith("BLU")) {
      drawColorPattern(options[i], textX, textY);
    } else {
      ucg.setColor(WHITE);
      ucg.setPrintPos(textX, textY);
      ucg.print(options[i]);
    }
  }
}

void generatePattern() {
  int type = random(1, 4 + difficulty/3); // Higher difficulty increases chance of mixed patterns
  String* source;
  int sourceLen;
  
  switch (type) {
    case 1:
      source = colors;
      sourceLen = 4;
      break;
    case 2:
      source = shapes;
      sourceLen = 5;
      break;
    case 3:
      source = stars;
      sourceLen = 5;
      break;
    default: // Mixed pattern
      // For mixed patterns, create a custom sequence
      pattern[0] = colors[random(0, 4)];
      int stepType = random(1, 3); // 1: alternate, 2: every third
      for (int i = 1; i < 5; i++) {
        if (i % stepType == 0) {
          // Change pattern type
          pattern[i] = shapes[random(0, 5)];
        } else {
          // Keep same pattern type but different value
          if (pattern[i-1].startsWith("RED") || pattern[i-1].startsWith("YEL") || 
              pattern[i-1].startsWith("GRN") || pattern[i-1].startsWith("BLU")) {
            pattern[i] = colors[random(0, 4)];
          } else if (pattern[i-1].startsWith("*")) {
            pattern[i] = stars[random(0, 5)];
          } else {
            pattern[i] = shapes[random(0, 5)];
          }
        }
      }
      
      // Create options based on last pattern element
      String lastType = pattern[4];
      currentAnswer = random(0, 4);  // Changed to 4 options
      
      for (int i = 0; i < 4; i++) {  // Changed to 4 options
        if (i == currentAnswer) {
          // For the correct answer, follow the pattern
          if (lastType.startsWith("RED") || lastType.startsWith("YEL") || 
              lastType.startsWith("GRN") || lastType.startsWith("BLU")) {
            options[i] = colors[random(0, 4)];
          } else if (lastType.startsWith("*")) {
            options[i] = stars[random(0, 5)];
          } else {
            options[i] = shapes[random(0, 5)];
          }
        } else {
          // For wrong answers, use a different type
          if (lastType.startsWith("RED") || lastType.startsWith("YEL") || 
              lastType.startsWith("GRN") || lastType.startsWith("BLU")) {
            options[i] = random(0, 2) ? shapes[random(0, 5)] : stars[random(0, 5)];
          } else if (lastType.startsWith("*")) {
            options[i] = random(0, 2) ? colors[random(0, 4)] : shapes[random(0, 5)];
          } else {
            options[i] = random(0, 2) ? colors[random(0, 4)] : stars[random(0, 5)];
          }
        }
      }
      return;
  }
  
  // Simple pattern - continuous sequence
  int startIndex = random(0, sourceLen - 3);
  int direction = random(0, 2) ? 1 : -1; // Forward or backward sequence
  
  for (int i = 0; i < 5; i++) {
    int idx = (startIndex + i * direction) % sourceLen;
    if (idx < 0) idx += sourceLen;
    pattern[i] = source[idx];
  }
  
  // Create next pattern element as correct answer
  int nextIdx = (startIndex + 5 * direction) % sourceLen;
  if (nextIdx < 0) nextIdx += sourceLen;
  
  currentAnswer = random(0, 4);  // Changed to 4 options
  for (int i = 0; i < 4; i++) {  // Changed to 4 options
    if (i == currentAnswer) {
      options[i] = source[nextIdx]; // Correct continuation
    } else {
      // Wrong answers should be different from correct
      int wrongIdx;
      do {
        wrongIdx = random(0, sourceLen);
      } while (wrongIdx == nextIdx);
      options[i] = source[wrongIdx];
    }
  }
}

void handleJoystick() {
  int xVal = analogRead(JOY_X);
  int yVal = analogRead(JOY_Y);
  
  if (millis() - lastJoystickMove > 300) {
    // Left
    if (xVal < 300 && selectedOption % 2 != 0) {
      selectedOption--;
      lastJoystickMove = millis();
      drawPattern();
    }
    // Right
    else if (xVal > 700 && selectedOption % 2 == 0 && selectedOption < 3) {
      selectedOption++;
      lastJoystickMove = millis();
      drawPattern();
    }
    // Up
    else if (yVal < 300 && selectedOption >= 2) {
      selectedOption -= 2;
      lastJoystickMove = millis();
      drawPattern();
    }
    // Down
    else if (yVal > 700 && selectedOption < 2) {
      selectedOption += 2;
      lastJoystickMove = millis();
      drawPattern();
    }
  }
  
  if (digitalRead(JOY_BTN) == LOW) {
    if (selectedOption == currentAnswer) {
      score++;
      playBuzzer(true);
      drawOverlay("Correct Answer", "Next");
      delay(1500);
      
      // Increase difficulty every 3 correct answers
      if (score % 3 == 0) {
        difficulty++;
        
        // Show level up message
        ucg.setColor(BLACK);
        ucg.drawBox(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT);
        ucg.setFont(ucg_font_helvR18_tr);
        ucg.setColor(YELLOW);
        ucg.setPrintPos(60, 100);
        ucg.print("LEVEL UP!");
        ucg.setFont(ucg_font_helvR14_tr);
        ucg.setPrintPos(80, 140);
        ucg.print("Level: ");
        ucg.print(difficulty);
        delay(2000);
      }
    } else {
      playBuzzer(false);
      drawOverlay("Wrong Answer", "Retry");
      delay(1500);
    }
    generatePattern();
    drawPattern();
    delay(500); // Debounce
  }
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(JOY_BTN, INPUT_PULLUP);
  
  Serial.begin(9600);
  randomSeed(micros());  // Better random seed using micros()
  
  // Initialize display
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.setRotate90(); // Landscape mode
  
  showSplashScreen();
  generatePattern();
  drawPattern();
}

void loop() {
  // Handle joystick input
  handleJoystick();
}