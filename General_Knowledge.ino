#ifndef FACTS_LEARNING_APP_H
#define FACTS_LEARNING_APP_H

#include <Ucglib.h>

// Define your display pins here
#define TFT_CS 17    // CS pin
#define TFT_DC 15    // DC/RS pin 
#define TFT_RST 16   // Reset pin (or connect to Arduino reset)

// Display object - update these pins to match your wiring
Ucglib_ILI9341_18x240x320_HWSPI ucg(TFT_DC, TFT_CS, TFT_RST);

#define JOY_X A0
#define JOY_Y A1
#define JOY_BTN 4
#define BUZZER_PIN 5

// Display is in landscape mode (320×240)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Color definitions
#define COLOR_BLACK 0, 0, 0
#define COLOR_GREEN 0, 255, 0
#define COLOR_RED 255, 0, 0  
#define COLOR_MAGENTA 255, 0, 255
#define COLOR_YELLOW 255, 255, 0
#define COLOR_WHITE 255, 255, 255
#define COLOR_CYAN 0, 255, 255
#define COLOR_BLUE 0, 0, 255

// ----- Facts Data -----
struct Fact {
  String question;
  String answer;
};

Fact allFacts[] = {
  {"What is the national animal of India?", "Tiger"},
  {"What is the national bird of India?", "Peacock"},
  {"Who was the first Prime Minister of India?", "Jawaharlal Nehru"},
  {"Who was the first President of India?", "Dr. Rajendra Prasad"},
  {"Which planet is known as the Red Planet?", "Mars"},
  {"What is H2O commonly known as?", "Water"},
  {"Which gas do we breathe in to survive?", "Oxygen"},
  {"What is the largest organ in the human body?", "Skin"},
  {"What do bees collect from flowers?", "Nectar"},
  {"How many continents are there?", "Seven"},
  {"Which country is known as the Land of the Rising Sun?", "Japan"},
  {"What color is chlorophyll?", "Green"},
  {"Who discovered gravity?", "Isaac Newton"},
  {"Which animal is known as the ship of the desert?", "Camel"},
  {"Which festival is known as the festival of lights?", "Diwali"},
  {"What is the capital of India?", "New Delhi"},
  {"How many days are there in a week?", "Seven"},
  {"Which shape has three sides?", "Triangle"},
  {"Which fruit is yellow and curved?", "Banana"},
  {"What do we wear on our feet?", "Shoes"},
  {"Who is known as the Father of the Nation (India)?", "Mahatma Gandhi"},
  {"What do cows give us?", "Milk"},
  {"Which month comes after March?", "April"},
  {"What is 5 + 3?", "8"},
  {"What is the color of the sky on a clear day?", "Blue"},
  {"What do we use to write on a blackboard?", "Chalk"},
  {"Which is the fastest land animal?", "Cheetah"},
  {"Which planet do we live on?", "Earth"},
  {"What do plants need to make food?", "Sunlight"}
};

const int totalFacts = sizeof(allFacts) / sizeof(allFacts[0]);

// ----- Quiz Config -----
#define MAX_OPTIONS 4
#define QUIZ_QUESTIONS 5
#define FACTS_BEFORE_QUIZ 10

struct QuizQuestion {
  String question;
  String correctAnswer;
  String options[MAX_OPTIONS];
  int correctIndex;
};

QuizQuestion quiz[QUIZ_QUESTIONS];
Fact recentFacts[FACTS_BEFORE_QUIZ];
int currentQuizQ = 0;
int selectedOption = 0;
int correctCount = 0;
int wrongCount = 0;

int currentFactIndex = 0;
int factsViewed = 0;
bool isNextSelected = true;

// -------- App States ---------
enum AppState {
  STATE_MENU,
  STATE_FACTS,
  STATE_QUIZ,
  STATE_QUIZ_RESULT
};

AppState currentState = STATE_MENU;
int menuSelection = 0;
const int menuItems = 1; // Only "Learn Facts" now

// ----------------------- Utility -----------------------
void shuffleFacts(Fact arr[], int size) {
  for (int i = size - 1; i > 0; i--) {
    int j = random(i + 1);
    Fact temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
  }
}

// ----------------- Menu Functions ---------------------
void drawMenu() {
  ucg.clearScreen();
  
  // Draw title
  ucg.setColor(COLOR_YELLOW);
  ucg.setFont(ucg_font_ncenR14_hr);
  ucg.setPrintPos(80, 40);
  ucg.print("FACTS LEARNER");
  
  // Draw menu options
  const char* menuOptions[] = {"Learn Facts"};
  
  for (int i = 0; i < menuItems; i++) {
    ucg.setPrintPos(80, 90 + i * 40);
    
    if (i == menuSelection) {
      ucg.setColor(COLOR_CYAN);
      ucg.print(">> ");
    } else {
      ucg.setColor(COLOR_WHITE);
      ucg.print("   ");
    }
    
    ucg.print(menuOptions[i]);
  }
}

void handleMenuInput(int joyY) {
  if (joyY < 400 && menuSelection > 0) {
    menuSelection--;
    drawMenu();
  } else if (joyY > 600 && menuSelection < menuItems - 1) {
    menuSelection++;
    drawMenu();
  }
}

void selectMenuItem() {
  currentState = STATE_FACTS;
  drawFactViewer();
}

// ------------------ Fact Navigation --------------------
void drawFactViewer() {
  ucg.clearScreen();
  
  // Set common font and colors for question
  ucg.setFont(ucg_font_ncenR14_tr);
  ucg.setColor(COLOR_WHITE);
  
  // Display question with word wrapping
  String question = allFacts[currentFactIndex].question;
  int textX = 15;
  int textY = 30;
  int lineHeight = 24;
  int maxCharsPerLine = 35;
  
  String currentLine = "";
  for (int i = 0; i < question.length(); i++) {
    currentLine += question[i];
    
    if (currentLine.length() >= maxCharsPerLine || 
        (question[i] == ' ' && currentLine.length() > maxCharsPerLine/2)) {
      ucg.setPrintPos(textX, textY);
      ucg.print(currentLine);
      textY += lineHeight;
      currentLine = "";
    }
  }
  
  if (currentLine.length() > 0) {
    ucg.setPrintPos(textX, textY);
    ucg.print(currentLine);
    textY += lineHeight;
  }
  
  // Add space before answer and change font
  textY += 20;
  ucg.setFont(ucg_font_ncenR18_tr);
  ucg.setColor(COLOR_YELLOW);
  
  // Center the answer
  String answer = allFacts[currentFactIndex].answer;
  int answerWidth = answer.length() * 12;
  int answerX = (SCREEN_WIDTH - answerWidth) / 2;
  answerX = max(answerX, textX); // Ensure doesn't go left of margin
  
  ucg.setPrintPos(answerX, textY);
  ucg.print(answer);
  
  // Navigation buttons at bottom
  int buttonY = SCREEN_HEIGHT - 30;
  ucg.setFont(ucg_font_ncenR14_tr);
  
  // Next button
  ucg.setColor(isNextSelected ? COLOR_CYAN : COLOR_WHITE);
  ucg.setPrintPos(40, buttonY);
  ucg.print(isNextSelected ? ">> Next" : "   Next");
  
  // Back button
  ucg.setColor(!isNextSelected ? COLOR_CYAN : COLOR_WHITE);
  ucg.setPrintPos(200, buttonY);
  ucg.print(!isNextSelected ? ">> Back" : "   Back");
  
  // Progress indicator
  ucg.setColor(COLOR_WHITE);
  ucg.setPrintPos(SCREEN_WIDTH - 60, SCREEN_HEIGHT - 10);
  ucg.print(currentFactIndex + 1);
  ucg.print("/");
  ucg.print(totalFacts);
}

void handleFactNavInput(int joyX) {
  if (joyX < 400) {
    isNextSelected = false;
    drawFactViewer();
  } else if (joyX > 600) {
    isNextSelected = true;
    drawFactViewer();
  }
}

void selectFactNav() {
  if (isNextSelected && factsViewed < FACTS_BEFORE_QUIZ) {
    recentFacts[factsViewed] = allFacts[currentFactIndex];
    factsViewed++;
  }

  if (isNextSelected) {
    currentFactIndex = (currentFactIndex + 1) % totalFacts;
    
    if (factsViewed >= FACTS_BEFORE_QUIZ) {
      prepareQuiz();
      currentState = STATE_QUIZ;
      drawQuizQuestion();
      factsViewed = 0;
      return;
    }
  } else {
    currentFactIndex = (currentFactIndex - 1 + totalFacts) % totalFacts;
    if (factsViewed > 0) factsViewed--;
  }
  drawFactViewer();
}

// --------------------- Quiz Logic ----------------------
void prepareQuiz() {
  for (int i = 0; i < QUIZ_QUESTIONS; i++) {
    int correctIdx = i % FACTS_BEFORE_QUIZ;
    quiz[i].question = recentFacts[correctIdx].question;
    quiz[i].correctAnswer = recentFacts[correctIdx].answer;
    quiz[i].options[0] = recentFacts[correctIdx].answer;

    for (int j = 1; j < MAX_OPTIONS; j++) {
      int randomWrong;
      bool duplicate;
      do {
        duplicate = false;
        randomWrong = random(FACTS_BEFORE_QUIZ);
        
        if (randomWrong == correctIdx) {
          duplicate = true;
          continue;
        }
        
        for (int k = 0; k < j; k++) {
          if (recentFacts[randomWrong].answer == quiz[i].options[k]) {
            duplicate = true;
            break;
          }
        }
      } while (duplicate);
      
      quiz[i].options[j] = recentFacts[randomWrong].answer;
    }

    // Shuffle options
    for (int k = 0; k < MAX_OPTIONS; k++) {
      int r = random(MAX_OPTIONS);
      String temp = quiz[i].options[k];
      quiz[i].options[k] = quiz[i].options[r];
      quiz[i].options[r] = temp;
    }

    // Find correct answer's new position
    for (int k = 0; k < MAX_OPTIONS; k++) {
      if (quiz[i].options[k] == quiz[i].correctAnswer) {
        quiz[i].correctIndex = k;
        break;
      }
    }
  }
  
  currentQuizQ = 0;
  selectedOption = 0;
  correctCount = 0;
  wrongCount = 0;
}

void drawQuizQuestion() {
  ucg.clearScreen();
  
  // Draw quiz header
  ucg.setColor(COLOR_YELLOW);
  ucg.setFont(ucg_font_ncenR14_hr);
  ucg.setPrintPos(10, 20);
  ucg.print("Quiz Question ");
  ucg.print(currentQuizQ + 1);
  ucg.print("/");
  ucg.print(QUIZ_QUESTIONS);

  // Draw question text with word wrap
  ucg.setColor(COLOR_WHITE);
  ucg.setFont(ucg_font_ncenR12_tr);
  
  String question = quiz[currentQuizQ].question;
  int textX = 10;
  int textY = 50;
  int charWidth = 8;
  int maxChars = (SCREEN_WIDTH - 20) / charWidth;
  
  String line = "";
  for (int i = 0; i < question.length(); i++) {
    line += question[i];
    
    if (line.length() >= maxChars || question[i] == ' ' && line.length() > maxChars/2) {
      ucg.setPrintPos(textX, textY);
      ucg.print(line);
      textY += 20;
      if (question[i] == ' ') line = "";
      else line = String(question[i]);
    }
  }
  
  if (line.length() > 0) {
    ucg.setPrintPos(textX, textY);
    ucg.print(line);
    textY += 20;
  }
  
  // Draw options
  textY += 10;
  for (int i = 0; i < MAX_OPTIONS; i++) {
    ucg.setPrintPos(20, textY + i * 30);
    
    if (i == selectedOption) {
      ucg.setColor(COLOR_CYAN);
      ucg.print(">> ");
    } else {
      ucg.setColor(COLOR_WHITE);
      ucg.print("   ");
    }
    
    ucg.print(quiz[currentQuizQ].options[i]);
  }
}

void handleQuizInput(int joyY) {
  if (joyY < 400 && selectedOption > 0) {
    selectedOption--;
    drawQuizQuestion();
  } else if (joyY > 600 && selectedOption < MAX_OPTIONS - 1) {
    selectedOption++;
    drawQuizQuestion();
  }
}

void handleQuizAnswer() {
  if (selectedOption == quiz[currentQuizQ].correctIndex) {
    correctCount++;
    tone(BUZZER_PIN, 800, 100);
    delay(100);
    tone(BUZZER_PIN, 1000, 100);
  } else {
    wrongCount++;
    tone(BUZZER_PIN, 400, 200);
    delay(100);
    tone(BUZZER_PIN, 300, 200);
  }

  currentQuizQ++;

  if (currentQuizQ >= QUIZ_QUESTIONS) {
    currentState = STATE_QUIZ_RESULT;
    drawQuizResult();
  } else {
    selectedOption = 0;
    drawQuizQuestion();
  }
}

// --------------------- Result --------------------------
void drawQuizResult() {
  ucg.clearScreen();
  
  // Header
  ucg.setColor(COLOR_YELLOW);
  ucg.setFont(ucg_font_ncenR14_hr);
  ucg.setPrintPos(80, 40);
  ucg.print("QUIZ RESULTS");
  
  // Show score
  ucg.setFont(ucg_font_ncenR12_tr);
  
  ucg.setPrintPos(60, 80);
  ucg.setColor(COLOR_GREEN);
  ucg.print("Correct: ");
  ucg.print(correctCount);
  
  ucg.setPrintPos(60, 110);
  ucg.setColor(COLOR_RED);
  ucg.print("Incorrect: ");
  ucg.print(wrongCount);
  
  ucg.setPrintPos(60, 150);
  ucg.setColor(COLOR_WHITE);
  ucg.print("Score: ");
  ucg.print((correctCount * 100) / QUIZ_QUESTIONS);
  ucg.print("%");
  
  // Options
  ucg.setColor(COLOR_CYAN);
  ucg.setPrintPos(60, 190);
  if (correctCount > 3) {
    ucg.print(">> Great Job!");
  } else {
    ucg.print(">> Try Again");
  }
  
  ucg.setColor(COLOR_WHITE);
  ucg.setPrintPos(60, 220);
  ucg.print("Press button for menu");
}

void handleQuizResult() {
  currentState = STATE_MENU;
  menuSelection = 0;
  drawMenu();
}

// ------------------- Setup & Loop ----------------------
void setup() {
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);
  pinMode(JOY_BTN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  
  randomSeed(analogRead(A2));
  
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  ucg.setRotate90();
  ucg.clearScreen();
  
  drawMenu();
}

void loop() {
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);
  bool buttonPressed = digitalRead(JOY_BTN) == LOW;
  
  switch (currentState) {
    case STATE_MENU:
      handleMenuInput(joyY);
      if (buttonPressed) {
        delay(200);
        selectMenuItem();
      }
      break;
      
    case STATE_FACTS:
      handleFactNavInput(joyX);
      if (buttonPressed) {
        delay(200);
        selectFactNav();
      }
      break;
      
    case STATE_QUIZ:
      handleQuizInput(joyY);
      if (buttonPressed) {
        delay(200);
        handleQuizAnswer();
      }
      break;
      
    case STATE_QUIZ_RESULT:
      if (buttonPressed) {
        delay(200);
        handleQuizResult();
      }
      break;
  }
  
  if (buttonPressed) {
    delay(1000);
    if (digitalRead(JOY_BTN) == LOW) {
      currentState = STATE_MENU;
      drawMenu();
      
      while (digitalRead(JOY_BTN) == LOW) {
        delay(10);
      }
    }
  }
  
  delay(100);
}

#endif