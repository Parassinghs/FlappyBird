/*
 ██████╗██╗      █████╗ ██████╗ ██████╗ ██╗   ██╗    ██████╗ ██╗██████╗ ██████╗
██╔════╝██║     ██╔══██╗██╔══██╗██╔══██╗╚██╗ ██╔╝    ██╔══██╗██║██╔══██╗██╔══██╗
██║     ██║     ███████║██████╔╝██████╔╝ ╚████╔╝     ██████╔╝██║██████╔╝██║  ██║
██║     ██║     ██╔══██║██╔═══╝ ██╔═══╝   ╚██╔╝      ██╔══██╗██║██╔══██╗██║  ██║
╚██████╗███████╗██║  ██║██║     ██║        ██║        ██████╔╝██║██║  ██║██████╔╝
 ╚═════╝╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝        ╚═╝        ╚═════╝ ╚═╝╚═╝  ╚═╝╚═════╝
  Colorful Terminal Edition — C++
  Controls: SPACE or ENTER to flap | Q to quit
*/

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <string>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#define CLEAR "cls"

// Fallback for older MinGW that lacks this constant
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

void enableVirtualTerminal()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
        return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
        return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
// Win32 Sleep() — no <thread>/<chrono> needed on Windows
inline void sleepMs(int ms) { Sleep((DWORD)ms); }
bool kbhit_check() { return (bool)_kbhit(); }
char getkey() { return (char)_getch(); }

#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <thread>
#include <chrono>
#define CLEAR "clear"

void enableVirtualTerminal() {}
inline void sleepMs(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

bool kbhit_check()
{
    struct termios oldt, newt;
    int ch, oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return true;
    }
    return false;
}
char getkey() { return getchar(); }
#endif

// ─── ANSI COLOR CODES ────────────────────────────────────────────────────────
#define RESET "\033[0m"
#define BOLD "\033[1m"

// Foreground colors
#define FG_BLACK "\033[30m"
#define FG_RED "\033[31m"
#define FG_GREEN "\033[32m"
#define FG_YELLOW "\033[33m"
#define FG_BLUE "\033[34m"
#define FG_MAGENTA "\033[35m"
#define FG_CYAN "\033[36m"
#define FG_WHITE "\033[37m"
#define FG_ORANGE "\033[38;5;214m"
#define FG_LIME "\033[38;5;154m"
#define FG_SKY "\033[38;5;117m"
#define FG_GOLD "\033[38;5;220m"
#define FG_PINK "\033[38;5;213m"

// Background colors
#define BG_BLACK "\033[40m"
#define BG_RED "\033[41m"
#define BG_GREEN "\033[42m"
#define BG_YELLOW "\033[43m"
#define BG_BLUE "\033[44m"
#define BG_CYAN "\033[46m"
#define BG_WHITE "\033[47m"
#define BG_SKYBLUE "\033[48;5;153m"
#define BG_DARKBLUE "\033[48;5;18m"
#define BG_PIPE "\033[48;5;28m"
#define BG_GROUND "\033[48;5;94m"

// ─── GAME CONSTANTS ───────────────────────────────────────────────────────────
const int WIDTH = 60;
const int HEIGHT = 22;
const int GROUND_ROW = HEIGHT - 2;
const int PIPE_WIDTH = 5;
const int GAP_SIZE = 7;
const int PIPE_SPEED = 1;
const double GRAVITY = 0.45;
const double FLAP_VEL = -1.8;
const int PIPE_INTERVAL = 25;

// ─── CURSOR HIDE/SHOW ─────────────────────────────────────────────────────────
void hideCursor() { std::cout << "\033[?25l"; }
void showCursor() { std::cout << "\033[?25h"; }
void moveCursor(int row, int col)
{
    std::cout << "\033[" << row << ";" << col << "H";
}

// ─── STRUCTURES ───────────────────────────────────────────────────────────────
struct Bird
{
    double y;
    double vel;
    int x;

    Bird() : y(HEIGHT / 2.0), vel(0.0), x(12) {}

    void flap() { vel = FLAP_VEL; }
    void update()
    {
        vel += GRAVITY;
        y += vel;
    }
    int row() const { return (int)y; }
};

struct Pipe
{
    int x;
    int gapTop; // top of the gap (inclusive)
    bool scored;

    Pipe(int startX) : x(startX), scored(false)
    {
        gapTop = 2 + rand() % (GROUND_ROW - GAP_SIZE - 3);
    }

    void update() { x -= PIPE_SPEED; }
    bool offScreen() const { return x + PIPE_WIDTH < 0; }

    // Returns true if the bird (at birdX, birdRow) collides with this pipe
    bool collides(int birdX, int birdRow) const
    {
        if (birdX + 1 < x || birdX > x + PIPE_WIDTH - 1)
            return false;
        // Within horizontal range — check vertical gap
        return (birdRow < gapTop || birdRow >= gapTop + GAP_SIZE);
    }
};

// ─── FRAME BUFFER ─────────────────────────────────────────────────────────────
struct Cell
{
    char ch;
    std::string color;
    std::string bg;

    Cell() : ch(' '), color(""), bg("") {}
    Cell(char c, std::string fg, std::string bg_c = "")
        : ch(c), color(fg), bg(bg_c) {}
};

Cell frame[HEIGHT][WIDTH];

void clearFrame()
{
    for (int r = 0; r < HEIGHT; r++)
        for (int c = 0; c < WIDTH; c++)
            frame[r][c] = Cell(' ', "", BG_SKYBLUE);
}

void setCell(int r, int c, char ch, const std::string &fg, const std::string &bg = "")
{
    if (r < 0 || r >= HEIGHT || c < 0 || c >= WIDTH)
        return;
    frame[r][c] = Cell(ch, fg, bg);
}

void renderFrame()
{
    moveCursor(1, 1);
    for (int r = 0; r < HEIGHT; r++)
    {
        for (int c = 0; c < WIDTH; c++)
        {
            Cell &cell = frame[r][c];
            std::string out = "";
            out += cell.bg;
            out += cell.color;
            out += cell.ch;
            out += RESET;
            std::cout << out;
        }
        std::cout << "\n";
    }
    std::cout.flush();
}

// ─── DRAW HELPERS ─────────────────────────────────────────────────────────────
void drawSky()
{
    // Gradient sky - top rows slightly darker
    for (int r = 0; r < GROUND_ROW; r++)
    {
        std::string skyBg = (r < 5) ? BG_DARKBLUE : BG_SKYBLUE;
        for (int c = 0; c < WIDTH; c++)
        {
            frame[r][c].bg = skyBg;
        }
    }
}

void drawClouds(int tick)
{
    // Simple scrolling cloud dots
    int cloudPositions[][2] = {{5, 3}, {20, 1}, {35, 4}, {50, 2}, {15, 6}, {45, 5}};
    for (auto &cp : cloudPositions)
    {
        int c = ((cp[0] - tick / 3) % WIDTH + WIDTH) % WIDTH;
        int r = cp[1];
        setCell(r, c, '~', FG_WHITE, BG_SKYBLUE);
        if (c + 1 < WIDTH)
            setCell(r, c + 1, '~', FG_WHITE, BG_SKYBLUE);
        if (c + 2 < WIDTH)
            setCell(r - 1 < 0 ? 0 : r - 1, c + 1, '*', FG_WHITE, BG_SKYBLUE);
    }
}

void drawGround()
{
    // Ground strip
    for (int c = 0; c < WIDTH; c++)
    {
        setCell(GROUND_ROW, c, '=', FG_YELLOW, BG_GROUND);
        setCell(GROUND_ROW + 1, c, '#', FG_GREEN, BG_GROUND);
    }
}

void drawPipe(const Pipe &p)
{
    // Pipe body
    for (int r = 0; r < p.gapTop; r++)
    {
        for (int c = p.x; c < p.x + PIPE_WIDTH; c++)
        {
            if (c >= 0 && c < WIDTH && r >= 0 && r < HEIGHT)
                setCell(r, c, '|', FG_LIME, BG_PIPE);
        }
    }
    // Top pipe cap
    int capRow = p.gapTop - 1;
    for (int c = p.x - 1; c < p.x + PIPE_WIDTH + 1; c++)
    {
        if (c >= 0 && c < WIDTH && capRow >= 0 && capRow < HEIGHT)
            setCell(capRow, c, '=', FG_GOLD, BG_PIPE);
    }

    // Bottom pipe body
    int botStart = p.gapTop + GAP_SIZE;
    for (int r = botStart; r < GROUND_ROW; r++)
    {
        for (int c = p.x; c < p.x + PIPE_WIDTH; c++)
        {
            if (c >= 0 && c < WIDTH && r >= 0 && r < HEIGHT)
                setCell(r, c, '|', FG_LIME, BG_PIPE);
        }
    }
    // Bottom pipe cap
    for (int c = p.x - 1; c < p.x + PIPE_WIDTH + 1; c++)
    {
        if (c >= 0 && c < WIDTH && botStart < HEIGHT)
            setCell(botStart, c, '=', FG_GOLD, BG_PIPE);
    }
}

void drawBird(const Bird &bird, int tick)
{
    int r = bird.row();
    int c = bird.x;

    // Bird body - choose wing char based on flap animation
    bool wingUp = (tick % 4 < 2);
    char wing = wingUp ? '^' : 'v';

    // Eye + beak
    setCell(r, c, '(', FG_ORANGE, "");
    setCell(r, c + 1, '@', FG_GOLD, "");
    setCell(r, c + 2, '>', FG_RED, "");
    setCell(r + 1, c, wing, FG_YELLOW, "");
    setCell(r + 1, c + 1, ')', FG_ORANGE, "");
}

void drawHUD(int score, int highScore)
{
    // Score bar at top
    moveCursor(HEIGHT + 2, 1);
    std::cout << BOLD << FG_CYAN
              << "  Score: " << FG_GOLD << score
              << FG_CYAN << "   |   Best: " << FG_PINK << highScore
              << FG_CYAN << "   |   [SPACE] Flap   [Q] Quit"
              << RESET << "        " << std::endl;
}

// ─── SCREENS ─────────────────────────────────────────────────────────────────
void clearScreen() { system(CLEAR); }

void showWelcomeScreen()
{
    clearScreen();
    hideCursor();
    std::cout << "\n\n";
    std::cout << BOLD << FG_YELLOW << "   ███████╗██╗      █████╗ ██████╗ ██████╗ ██╗   ██╗\n"
              << RESET;
    std::cout << BOLD << FG_ORANGE << "   ██╔════╝██║     ██╔══██╗██╔══██╗██╔══██╗╚██╗ ██╔╝\n"
              << RESET;
    std::cout << BOLD << FG_RED << "   █████╗  ██║     ███████║██████╔╝██████╔╝ ╚████╔╝ \n"
              << RESET;
    std::cout << BOLD << FG_MAGENTA << "   ██╔══╝  ██║     ██╔══██║██╔═══╝ ██╔═══╝   ╚██╔╝  \n"
              << RESET;
    std::cout << BOLD << FG_CYAN << "   ██║     ███████╗██║  ██║██║     ██║        ██║   \n"
              << RESET;
    std::cout << BOLD << FG_BLUE << "   ╚═╝     ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝        ╚═╝   \n"
              << RESET;
    std::cout << "\n";
    std::cout << BOLD << FG_SKY << "            ██████╗ ██╗██████╗ ██████╗\n"
              << RESET;
    std::cout << BOLD << FG_LIME << "            ██╔══██╗██║██╔══██╗██╔══██╗\n"
              << RESET;
    std::cout << BOLD << FG_GREEN << "            ██████╔╝██║██████╔╝██║  ██║\n"
              << RESET;
    std::cout << BOLD << FG_GOLD << "            ██╔══██╗██║██╔══██╗██║  ██║\n"
              << RESET;
    std::cout << BOLD << FG_ORANGE << "            ██████╔╝██║██║  ██║██████╔╝\n"
              << RESET;
    std::cout << BOLD << FG_RED << "            ╚═════╝ ╚═╝╚═╝  ╚═╝╚═════╝ \n"
              << RESET;
    std::cout << "\n";
    std::cout << "         " << FG_YELLOW << "( @ > " << RESET << "   " << FG_CYAN << "Colorful Terminal Edition\n"
              << RESET;
    std::cout << "\n";
    std::cout << "   " << BOLD << FG_WHITE << "Press " << FG_GOLD << "[SPACE]" << FG_WHITE << " or " << FG_GOLD << "[ENTER]" << FG_WHITE << " to start!\n"
              << RESET;
    std::cout << "   " << FG_MAGENTA << "Press [Q] anytime to quit.\n"
              << RESET;
    std::cout << "\n";
    std::cout << "   " << FG_CYAN << "Controls: SPACE / ENTER = Flap | Q = Quit\n"
              << RESET;

    // Wait for input
    while (true)
    {
        if (kbhit_check())
        {
            char key = getkey();
            if (key == ' ' || key == '\n' || key == '\r')
                break;
            if (key == 'q' || key == 'Q')
            {
                showCursor();
                exit(0);
            }
        }
        sleepMs(50);
    }
}

void showGameOver(int score, int highScore)
{
    // Overlay game over on top of frame
    moveCursor(HEIGHT / 2 - 2, 10);
    std::cout << BOLD << BG_RED << FG_WHITE << "                                    " << RESET;
    moveCursor(HEIGHT / 2 - 1, 10);
    std::cout << BOLD << BG_RED << FG_YELLOW << "         GAME  OVER!  :(             " << RESET;
    moveCursor(HEIGHT / 2, 10);
    std::cout << BOLD << BG_RED << FG_WHITE << "   Score: " << score << "   Best: " << highScore << "          " << RESET;
    moveCursor(HEIGHT / 2 + 1, 10);
    std::cout << BOLD << BG_RED << FG_CYAN << "   SPACE = Play Again  |  Q = Quit   " << RESET;
    moveCursor(HEIGHT / 2 + 2, 10);
    std::cout << BOLD << BG_RED << FG_WHITE << "                                    " << RESET;
    std::cout.flush();
}

// ─── MAIN GAME LOOP ───────────────────────────────────────────────────────────
int main()
{
    enableVirtualTerminal();
    srand((unsigned)time(nullptr));

    hideCursor();
    showWelcomeScreen();

    int highScore = 0;

    while (true)
    { // Outer restart loop
        // ── Init game state ──────────────────────────────────────────────────
        Bird bird;
        std::vector<Pipe> pipes;
        int score = 0;
        int tick = 0;
        bool alive = true;
        bool gameOver = false;

        clearScreen();

        // ── Game loop ────────────────────────────────────────────────────────
        while (!gameOver)
        {

            // -- Input --------------------------------------------------------
            if (kbhit_check())
            {
                char key = getkey();
                if (key == 'q' || key == 'Q')
                {
                    showCursor();
                    clearScreen();
                    std::cout << FG_CYAN << "\nThanks for playing Flappy Bird!\n"
                              << FG_GOLD << "Your best score: " << highScore
                              << RESET << "\n\n";
                    return 0;
                }
                if ((key == ' ' || key == '\n' || key == '\r') && alive)
                {
                    bird.flap();
                }
            }

            // -- Update -------------------------------------------------------
            if (alive)
            {
                bird.update();

                // Spawn pipes
                if (tick % PIPE_INTERVAL == 0)
                {
                    pipes.emplace_back(WIDTH);
                }

                // Move pipes
                for (auto &p : pipes)
                {
                    p.update();

                    // Score
                    if (!p.scored && p.x + PIPE_WIDTH < bird.x)
                    {
                        p.scored = true;
                        score++;
                        if (score > highScore)
                            highScore = score;
                    }
                }

                // Remove off-screen pipes
                pipes.erase(
                    std::remove_if(pipes.begin(), pipes.end(),
                                   [](const Pipe &p)
                                   { return p.offScreen(); }),
                    pipes.end());

                // Collision: ground / ceiling
                if (bird.row() >= GROUND_ROW || bird.row() < 0)
                {
                    alive = false;
                }

                // Collision: pipes
                for (auto &p : pipes)
                {
                    if (p.collides(bird.x, bird.row()))
                    {
                        alive = false;
                        break;
                    }
                }
            }

            // -- Render -------------------------------------------------------
            clearFrame();
            drawSky();
            drawClouds(tick);
            drawGround();

            for (auto &p : pipes)
                drawPipe(p);
            if (alive)
                drawBird(bird, tick);

            renderFrame();
            drawHUD(score, highScore);

            if (!alive)
            {
                showGameOver(score, highScore);
                gameOver = false; // keep showing until input

                // Wait for restart or quit
                bool waiting = true;
                while (waiting)
                {
                    if (kbhit_check())
                    {
                        char k = getkey();
                        if (k == ' ' || k == '\n' || k == '\r')
                        {
                            waiting = false;
                            gameOver = true; // break outer render loop → restart
                        }
                        if (k == 'q' || k == 'Q')
                        {
                            showCursor();
                            clearScreen();
                            std::cout << FG_CYAN << "\nThanks for playing!\n"
                                      << FG_GOLD << "Best score: " << highScore
                                      << RESET << "\n\n";
                            return 0;
                        }
                    }
                    sleepMs(50);
                }
                break; // go back to outer restart loop
            }

            tick++;

            // -- Frame timing: ~60ms per tick (~16 FPS) ----------------------
            sleepMs(60);
        }
    }

    showCursor();
    return 0;
}