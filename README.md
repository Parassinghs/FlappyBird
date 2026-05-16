# FlappyBird
========================================================
   FLAPPY BIRD — Colorful Terminal Edition (C++)
   README & Documentation
========================================================

--------------------------------------------------------
  ABOUT THE GAME
--------------------------------------------------------
This is a terminal-based Flappy Bird clone written in C++.
It runs entirely inside your command-line / terminal window
using ANSI escape codes to produce colorful, animated visuals
without any external game engine or graphics library.

The game features:
  - A colorful sky gradient (dark blue top, sky blue middle)
  - Scrolling white clouds
  - Animated flapping bird ( @ > ) in orange, gold & red
  - Bright lime-green pipes with gold caps
  - Yellow & green ground strip
  - A live score HUD at the bottom
  - A rainbow ASCII art welcome screen
  - A red Game Over overlay with your score & best score
  - Persistent high score during the session

--------------------------------------------------------
  FILE STRUCTURE
--------------------------------------------------------
  flappy_bird.cpp   — Full game source code (single file)
  README.txt        — This file

--------------------------------------------------------
  REQUIREMENTS
--------------------------------------------------------
  Compiler : g++ (MinGW on Windows, GCC on Linux/macOS)
  Standard : C++17  (-std=c++17)
  Terminal : Must support ANSI color codes

  Windows  — Use VS Code Terminal, Windows Terminal,
              or PowerShell. Avoid plain CMD.exe.
  Linux    — Any standard terminal (GNOME Terminal,
              Konsole, xterm, etc.)
  macOS    — Terminal.app or iTerm2

--------------------------------------------------------
  HOW TO COMPILE
--------------------------------------------------------

  [ Windows (MinGW / VS Code) ]
  ------------------------------
  Open the VS Code integrated terminal, then:

    g++ -std=c++17 -o flappy_bird.exe flappy_bird.cpp

  Run it:
    flappy_bird.exe

  [ Linux / macOS ]
  ------------------
  Open a terminal, then:

    g++ -std=c++17 -o flappy_bird flappy_bird.cpp

  Run it:
    ./flappy_bird

  [ VS Code Task (optional) ]
  ----------------------------
  You can add this to your .vscode/tasks.json to build
  with Ctrl+Shift+B:

    {
      "label": "Build Flappy Bird",
      "type": "shell",
      "command": "g++",
      "args": ["-std=c++17", "-o", "flappy_bird", "flappy_bird.cpp"],
      "group": { "kind": "build", "isDefault": true }
    }

--------------------------------------------------------
  CONTROLS
--------------------------------------------------------
  SPACE       — Flap (make the bird fly up)
  ENTER       — Flap (alternative key)
  Q           — Quit the game at any time
  SPACE       — Restart after Game Over

--------------------------------------------------------
  GAMEPLAY
--------------------------------------------------------
  1. Press SPACE or ENTER on the welcome screen to begin.
  2. The bird falls due to gravity automatically.
  3. Press SPACE or ENTER to flap and fly upward.
  4. Navigate through the gaps between the green pipes.
  5. Each pipe you pass through earns +1 point.
  6. The game ends if the bird:
       - Hits the top of the screen
       - Hits the ground
       - Collides with a pipe
  7. After Game Over, press SPACE to play again.
  8. Your best score is saved for the session.

--------------------------------------------------------
  GAME CONSTANTS (tunable in source code)
--------------------------------------------------------
  WIDTH           = 60       Terminal columns used
  HEIGHT          = 22       Terminal rows used
  GROUND_ROW      = 20       Row where ground starts
  PIPE_WIDTH      = 5        Width of each pipe
  GAP_SIZE        = 7        Vertical gap between pipes
  PIPE_SPEED      = 1        Pixels pipes move per tick
  GRAVITY         = 0.45     Downward pull on the bird
  FLAP_VEL        = -1.8     Upward velocity on flap
  PIPE_INTERVAL   = 25       Ticks between pipe spawns
  Frame delay     = 60ms     (~16 FPS)

  You can open flappy_bird.cpp and change these values
  at the top of the GAME CONSTANTS section to adjust
  difficulty and feel.

--------------------------------------------------------
  TROUBLESHOOTING
--------------------------------------------------------

  Problem : Colors/boxes look like garbled symbols
  Fix     : Switch to Windows Terminal or VS Code terminal.
            Plain CMD.exe may not support ANSI codes on
            older Windows versions.

  Problem : 'g++' is not recognized
  Fix     : Install MinGW-w64 and add it to your PATH.
            Guide: https://www.mingw-w64.org/

  Problem : ENABLE_VIRTUAL_TERMINAL_PROCESSING error
  Fix     : Already handled in code with a fallback define.
            Make sure you are using g++ with -std=c++17.

  Problem : Game runs too fast / too slow
  Fix     : Change the sleepMs(60) value near the bottom
            of the main game loop in flappy_bird.cpp.
            Higher = slower, Lower = faster.

  Problem : Screen flickers badly
  Fix     : Maximize your terminal window and use a font
            size where 60 columns fits comfortably.

--------------------------------------------------------
  HOW IT WORKS (Technical Overview)
--------------------------------------------------------
  - The game uses a double-buffer approach: every frame,
    a 2D array of Cell structs (char + ANSI color codes)
    is filled, then rendered to the terminal at once.

  - Cursor is hidden during gameplay (\033[?25l) and
    restored on exit to avoid visual clutter.

  - ANSI escape codes are used for:
      \033[Nm        — foreground / background colors
      \033[row;colH  — cursor positioning
      \033[?25l/h    — hide / show cursor

  - On Windows: Win32 Sleep() is used for frame timing.
  - On Linux/macOS: std::this_thread::sleep_for() is used.

  - Non-blocking keyboard input:
      Windows: _kbhit() + _getch() from <conio.h>
      Linux:   termios raw mode + fcntl O_NONBLOCK

--------------------------------------------------------
  AUTHOR & LICENSE
--------------------------------------------------------
  Created with C++ for educational and fun purposes.
  Free to use, modify, and share.
  No external dependencies — just a C++17 compiler!

========================================================
  Happy Flapping!   ( @ >  ~~~
========================================================
