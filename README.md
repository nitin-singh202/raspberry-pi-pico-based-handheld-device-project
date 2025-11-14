# raspberry-pi-pico-based-handheld-device-project
# 🕹️ Handheld Device Project – Raspberry Pi Pico

![Made with Arduino](https://img.shields.io/badge/Made%20with-Arduino-blue?logo=arduino)
![Platform](https://img.shields.io/badge/Platform-Raspberry%20Pi%20Pico-green?logo=raspberrypi)
![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![Status](https://img.shields.io/badge/Status-Completed-success)

---

## 📸 Device Preview

![Handheld Device Photo](images/device.jpg)

## 🎯 Overviews

The Handheld Device Project is a multifunctional portable gaming and utility console built using the Raspberry Pi Pico microcontroller. It features a TFT display, keypad/joystick input, and a suite of custom-made mini-games and utilities — all designed to run smoothly on embedded hardware.

Developed for the WAVES 2025 – Create in India Challenge, this project demonstrates embedded programming, electronics integration, and creative software design.

## 🏆 Recognition

🏅 **Selected Among the Top 7 Teams Nationwide** in the Create in India Challenge (WAVES 2025) for innovation, creativity, and technical excellence.

## ⚙️ Hardware Components

| Component | Description |
|-----------|-------------|
| Microcontroller | Raspberry Pi Pico |
| Display | ILI9341 TFT 240×320 (SPI) |
| Input | 4x4 Keypad / Joystick |
| Audio | Buzzer (optional) |
| Power Source | USB / Li-ion Battery |
| Graphics Library | Ucglib |

## 🧩 Features & Modules

This handheld device includes multiple standalone applications, each designed as a separate `.ino` file for modularity.

# 🎮 Game Collection

## 🎮 Games

### SnakeGame
[![Watch the video](https://img.youtube.com/vi/YOUTUBE_VIDEO_ID_1/maxresdefault.jpg)](https://youtube.com/link-to-snake-game)
**Description:** Classic snake game where you control a snake to eat food and grow longer.

### PingPong
[![Watch the video](https://img.youtube.com/vi/YOUTUBE_VIDEO_ID_2/maxresdefault.jpg)](https://youtube.com/link-to-pingpong)
**Description:** Two-player ping pong game with paddles and a ball.

### MazeNavigation
[![Watch the video](https://img.youtube.com/vi/YOUTUBE_VIDEO_ID_3/maxresdefault.jpg)](https://youtube.com/shorts/U3ZgrMS9GaI?feature=share)
**Description:** Navigate through complex mazes to reach the destination.

### BlastTheBomb
[![Watch the video](https://img.youtube.com/vi/YOUTUBE_VIDEO_ID_4/maxresdefault.jpg)](https://studio.youtube.com/video/Q6VPFCoEAoA/edit))
**Description:** Exciting bomb blasting game with timers and challenges.

## 🧠 Brain & Logic

### MathGame
[![Watch the video](https://img.youtube.com/vi/YOUTUBE_VIDEO_ID_5/maxresdefault.jpg)](https://youtube.com/shorts/Q6VPFCoEAoA)
**Description:** Test your math skills with various arithmetic challenges.

### PatternRecognition
**Video Link:** [Watch on YouTube](https://youtube.com/shorts/Kp-aqM4MeuY?feature=share)
**Description:** Identify patterns and sequences in this brain-teasing game.

### PuzzleSolving
**Video Link:** [Watch on YouTube](https://youtube.com/link-to-puzzle-game)
**Description:** Solve various puzzles and brain teasers.

### OddOneOut
**Video Link:** [Watch on YouTube](https://youtube.com/shorts/_VZd1BeF_88?feature=share)
**Description:** Find the item that doesn't belong in the group.

## 🧩 Memory & Sequence

### MemoryGame
**Video Link:** [Watch on YouTube](https://youtube.com/link-to-memory-game)
**Description:** Test your memory by matching pairs of cards.

### NumberMemory
**Video Link:** [Watch on YouTube](https://youtube.com/shorts/q4jkph3xYqg?feature=share)
**Description:** Remember and recall sequences of numbers.

### NumberSequencing
**Video Link:** [Watch on YouTube](https://youtube.com/shorts/syd2BUAB_fk?feature=share)
**Description:** Arrange numbers in the correct sequence.

## 🧮 Utilities

### Calculator
**Video Link:** [Watch on YouTube](https://youtube.com/shorts/nc7ObsH7d0c)
**Description:** A fully functional calculator application.

### SudokuGame
**Video Link:** [Watch on YouTube](https://youtube.com/link-to-sudoku)
**Description:** Classic Sudoku puzzle game with multiple difficulty levels.

### WordProblem
**Video Link:** [Watch on YouTube](https://youtube.com/link-to-word-problem)
**Description:** Solve word-based math and logic problems.

### GeneralKnowledge
**Video Link:** [Watch on YouTube](https://youtube.com/shorts/54KsS_FzODQ?feature=share)
**Description:** Test your general knowledge with trivia questions.
Each module runs independently and can be accessed from a Main Menu UI rendered on the TFT screen.

## 🖥️ Interface & Navigation

- Colorful main menu interface with highlight-based selection
- Keypad / Joystick navigation support
- Transition effects between menu and games
- Return to Menu option available in every module

## 🔧 Software Architecture

- Each `.ino` file defines its own `startGame()` (or similar entry function)
- A single main `.ino` file handles setup, loop, and menu navigation
- Shared display and input logic used across all modules
- Modular design for clean compilation and easy expansion

## 🚀 Getting Started

### 🧭 1. Clone the Repository
```bash
git clone https://github.com/nitin-singh202/raspberry-pi-pico-based-handheld-device-project
```
### **⚙️ 2. Open in Arduino IDE**
Ensure all .ino files are in one folder

Select Raspberry Pi Pico as the target board

Install the following libraries:

Ucglib

Adafruit_GFX

Adafruit_ILI9341 (if you use that for display control)

### **🔌 3. Hardware Setup**
Component	Pin

TFT CS	GP17
TFT DC	GP15
TFT RST	GP16
TFT SCK	GP18
TFT MOSI	GP19
TFT LED	3.3V
Keypad / Joystick	GP2–GP9
Power	3.3V / GND

### **⬆️ 4. Upload the Code**
Open the main file containing setup() and loop()

Click Upload in Arduino IDE

The device will boot into the Main Menu, ready for use! ⚡

## **🎨 Future Enhancements**
Animated transitions between games

Audio feedback for inputs and gameplay

Battery level indicator

Save high scores to EEPROM

Add multiplayer support using UART

## **🏅 Achievements**
🧠 Created 15+ original games and utilities for Pico

🎨 Developed custom UI using Ucglib

🔌 Integrated hardware input and TFT graphics

🏆 Recognized among Top 7 teams nationwide (WAVES 2025)

## **📄 License**
This project is released under the MIT License.
You're free to modify, share, and improve it — just give credit to the original author.

## **👨‍💻 Author**
Nitin
🎓 Computer Science Engineering Student
💡 Passionate about Embedded Systems, IoT, and Creative Hardware–Software Design
      
🌐 GitHub: github.com/nitin-singh202
📧 Email: nitinkumarsingh296@gmail.com

🌟 Support
If you like this project, please ⭐ star the repository on GitHub!
It helps support future improvements and new embedded creations.

