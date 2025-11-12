# raspberry-pi-pico-based-handheld-device-project
# 🕹️ Handheld Device Project – Raspberry Pi Pico

![Made with Arduino](https://img.shields.io/badge/Made%20with-Arduino-blue?logo=arduino)
![Platform](https://img.shields.io/badge/Platform-Raspberry%20Pi%20Pico-green?logo=raspberrypi)
![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![Status](https://img.shields.io/badge/Status-Completed-success)

---

## 📸 Device Preview

![Handheld Device Photo](images/device.jpg)

## 🎯 Overview

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

### 🎮 Games
- **SnakeGame**
- **PingPong** 
- **MazeNavigation**
- **BlastTheBomb**

### 🧠 Brain & Logic
- **MathGame**
- **PatternRecognition**
- **PuzzleSolving**
- **OddOneOut**

### 🧩 Memory & Sequence
- **MemoryGame**
- **NumberMemory**
- **NumberSequencing**

### 🧮 Utilities
- **Calculator**
- **SudokuGame**
- **WordProblem**
- **GeneralKnowledge**

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
git clone https://github.com/<your-username>/<repo-name>.git
```
###⚙️ 2. Open in Arduino IDE
Ensure all .ino files are in one folder

Select Raspberry Pi Pico as the target board

Install the following libraries:

Ucglib

Adafruit_GFX

Adafruit_ILI9341 (if you use that for display control)

###🔌 3. Hardware Setup
Component	Pin
TFT CS	GP17
TFT DC	GP15
TFT RST	GP16
TFT SCK	GP18
TFT MOSI	GP19
TFT LED	3.3V
Keypad / Joystick	GP2–GP9
Power	3.3V / GND
###⬆️ 4. Upload the Code
Open the main file containing setup() and loop()

Click Upload in Arduino IDE

The device will boot into the Main Menu, ready for use! ⚡

##🎨 Future Enhancements
Animated transitions between games

Audio feedback for inputs and gameplay

Battery level indicator

Save high scores to EEPROM

Add multiplayer support using UART

##🏅 Achievements
🧠 Created 15+ original games and utilities for Pico

🎨 Developed custom UI using Ucglib

🔌 Integrated hardware input and TFT graphics

🏆 Recognized among Top 7 teams nationwide (WAVES 2025)

##📄 License
This project is released under the MIT License.
You're free to modify, share, and improve it — just give credit to the original author.

##👨‍💻 Author
Nitin
🎓 Computer Science Engineering Student
💡 Passionate about Embedded Systems, IoT, and Creative Hardware–Software Design

🌐 GitHub: github.com/nitin-singh202
📧 Email: nitinkumarsingh296@gmail.com

🌟 Support
If you like this project, please ⭐ star the repository on GitHub!
It helps support future improvements and new embedded creations.

