#include <SFML/Graphics.hpp>
#include <time.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

using namespace sf;
using namespace std;

// CONSTANTS
const int M = 20;
const int N = 10;
const int TILE_SIZE = 30;
const int WIDTH = N * TILE_SIZE;
const int SIDEBAR_WIDTH = 250;
const int HEIGHT = M * TILE_SIZE;
const int TOTAL_WIDTH = WIDTH + SIDEBAR_WIDTH;

// GLOBALS
int field[M][N] = { 0 };

int figures[7][4] = {
    {1,3,5,7}, // I
    {2,4,5,7}, // Z
    {3,5,4,6}, // S
    {2,3,4,5}, // O 
    {3,5,4,7}, // T
    {2,3,5,7}, // L
    {3,5,7,6}, // J
};

struct GamePoint { int x, y; } a[4], b[4], shadow[4];

enum GameState { MENU, PLAY, PAUSE, GAMEOVER, HIGHSCORE, HELP };
GameState currentState = MENU;

int score = 0;
int level = 1;
int linesClearedTotal = 0;
float timer = 0, delay = 0.5f;
bool rotateFlag = false;
int colorNum = 1;
bool showText = true;

// To Show Next Piece
int nextColorNum = 1;
int nextFigureIdx = 0;

Clock difficultyClock;
int playableRows = M;

// FUNCTIONS

void saveHighScore(int newScore) {
    vector<int> scores;
    ifstream in("highscores.txt");
    int s;
    if (in.is_open()) {
        while (in >> s) scores.push_back(s);
        in.close();
    }

    scores.push_back(newScore);
    sort(scores.rbegin(), scores.rend());
    if (scores.size() > 10) scores.resize(10);

    ofstream out("highscores.txt");
    for (int i : scores) out << i << "\n";
    out.close();
}

vector<int> loadHighScores() {
    vector<int> scores;
    ifstream in("highscores.txt");
    int s;
    if (in.is_open()) {
        while (in >> s) scores.push_back(s);
    }
    while (scores.size() < 10) scores.push_back(0);
    return scores;
}

bool check() {
    for (int i = 0; i < 4; i++) {
        if (a[i].x < 0 || a[i].x >= N || a[i].y >= playableRows) return 0;
        if (field[a[i].y][a[i].x]) return 0;
    }
    return 1;
}

void resetGame() {
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++) field[i][j] = 0;

    score = 0;
    level = 1;
    linesClearedTotal = 0;
    delay = 0.5f;
    playableRows = M;
    difficultyClock.restart();

    int maxShapes = (level == 1) ? 4 : 7;
    int n = rand() % maxShapes;
    colorNum = 1 + rand() % 7;
    for (int i = 0; i < 4; i++) {
        a[i].x = figures[n][i] % 2 + N / 2 - 1;
        a[i].y = figures[n][i] / 2;
    }

    nextFigureIdx = rand() % maxShapes;
    nextColorNum = 1 + rand() % 7;
}

void centerText(Text& txt, float x, float width) {
    FloatRect textRect = txt.getLocalBounds();
    txt.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    txt.setPosition(x + width / 2.0f, txt.getPosition().y);
}

int main() {
    srand((unsigned int)time(0));
    RenderWindow window(VideoMode(TOTAL_WIDTH, HEIGHT), "Tetris Project Fall 2025");

    Font font;
    if (!font.loadFromFile("arial.ttf") &&
        !font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf") &&
        !font.loadFromFile("C:/Windows/Fonts/arial.ttf") &&
        !font.loadFromFile("C:\\Windows\\Fonts\\segoeui.ttf")) {
        showText = false;
    }

    Text titleText("", font, 35);
    titleText.setFillColor(Color::Cyan);
    titleText.setStyle(Text::Bold);

    Text normalText("", font, 20);
    normalText.setFillColor(Color::White);

    Clock clock;
    resetGame();

    while (window.isOpen()) {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        timer += time;

        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) window.close();

            if (e.type == Event::KeyPressed) {
                if (currentState == MENU) {
                    if (e.key.code == Keyboard::Num1) { resetGame(); currentState = PLAY; }
                    if (e.key.code == Keyboard::Num2) currentState = HIGHSCORE;
                    if (e.key.code == Keyboard::Num3) currentState = HELP;
                    if (e.key.code == Keyboard::Num4) window.close();
                }
                else if (currentState == PLAY) {
                    if (e.key.code == Keyboard::Up || e.key.code == Keyboard::W) {
                        rotateFlag = true;
                    }
                    else if (e.key.code == Keyboard::Left || e.key.code == Keyboard::A) {
                        for (int i = 0; i < 4; i++) { b[i] = a[i]; a[i].x--; }
                        if (!check()) for (int i = 0; i < 4; i++) a[i] = b[i];
                    }
                    else if (e.key.code == Keyboard::Right || e.key.code == Keyboard::D) {
                        for (int i = 0; i < 4; i++) { b[i] = a[i]; a[i].x++; }
                        if (!check()) for (int i = 0; i < 4; i++) a[i] = b[i];
                    }
                    else if (e.key.code == Keyboard::Down || e.key.code == Keyboard::S) {
                        delay = 0.05f;
                    }
                    else if (e.key.code == Keyboard::P) {
                        currentState = PAUSE;
                    }
                    else if (e.key.code == Keyboard::Space) {
                        while (true) {
                            for (int i = 0; i < 4; i++) b[i] = a[i];
                            for (int i = 0; i < 4; i++) a[i].y++;
                            if (!check()) {
                                for (int i = 0; i < 4; i++) a[i] = b[i];
                                break;
                            }
                        }
                        delay = 0;
                    }
                }
                else if (currentState == PAUSE) {
                    if (e.key.code == Keyboard::Num1) currentState = PLAY;

                    if (e.key.code == Keyboard::Num2 || e.key.code == Keyboard::Enter) {
                        resetGame();
                        currentState = PLAY;
                    }

                    if (e.key.code == Keyboard::Num3) currentState = HIGHSCORE;

                    if (e.key.code == Keyboard::Num4) window.close();

                    if (e.key.code == Keyboard::P) currentState = PLAY;
                }
                else if (currentState == GAMEOVER || currentState == HIGHSCORE || currentState == HELP) {
                    if (e.key.code == Keyboard::Escape) currentState = MENU;
                }
            }
            if (e.type == Event::KeyReleased) {
                if (e.key.code == Keyboard::Down || e.key.code == Keyboard::S) {
                    delay = (level == 2) ? 0.3f : 0.5f;
                }
            }
        }

        if (currentState == PLAY) {
            if (difficultyClock.getElapsedTime().asSeconds() > 300) {
                if (playableRows > 5) playableRows--;
                difficultyClock.restart();
            }

            if (rotateFlag) {
                GamePoint p = a[1];
                for (int i = 0; i < 4; i++) {
                    int x = a[i].y - p.y;
                    int y = a[i].x - p.x;
                    a[i].x = p.x - x;
                    a[i].y = p.y + y;
                }
                if (!check()) for (int i = 0; i < 4; i++) a[i] = b[i];
                rotateFlag = false;
            }

            if (timer > delay) {
                for (int i = 0; i < 4; i++) { b[i] = a[i]; a[i].y++; }

                if (!check()) {
                    for (int i = 0; i < 4; i++) field[b[i].y][b[i].x] = colorNum;

                    int linesCleared = 0;
                    int k = M - 1;
                    for (int i = M - 1; i > 0; i--) {
                        int count = 0;
                        for (int j = 0; j < N; j++) {
                            if (field[i][j]) count++;
                            field[k][j] = field[i][j];
                        }
                        if (count < N) k--;
                        else linesCleared++;
                    }

                    if (linesCleared > 0) {
                        linesClearedTotal += linesCleared;
                        int points = 0;
                        if (linesCleared == 1) points = 10;
                        else if (linesCleared == 2) points = 30;
                        else if (linesCleared == 3) points = 60;
                        else if (linesCleared == 4) points = 100;

                        score += points * level;
                    }

                    if (linesClearedTotal > 5 && level == 1) {
                        level = 2;
                        delay = 0.3f;
                    }

                    for (int i = 0; i < 4; i++) {
                        a[i].x = figures[nextFigureIdx][i] % 2 + N / 2 - 1;
                        a[i].y = figures[nextFigureIdx][i] / 2;
                    }
                    colorNum = nextColorNum;

                    int maxShapes = (level == 1) ? 4 : 7;
                    nextFigureIdx = rand() % maxShapes;
                    nextColorNum = 1 + rand() % 7;

                    if (!check()) {
                        saveHighScore(score);
                        currentState = GAMEOVER;
                    }
                }
                timer = 0;
            }

            for (int i = 0; i < 4; i++) shadow[i] = a[i];
            while (true) {
                for (int i = 0; i < 4; i++) shadow[i].y++;
                bool collision = false;
                for (int i = 0; i < 4; i++) {
                    if (shadow[i].x < 0 || shadow[i].x >= N || shadow[i].y >= playableRows || field[shadow[i].y][shadow[i].x])
                        collision = true;
                }
                if (collision) { for (int i = 0; i < 4; i++) shadow[i].y--; break; }
            }
        }

        window.clear(Color::Black);

        if (showText && currentState == MENU) {
            RectangleShape bg(Vector2f(TOTAL_WIDTH, HEIGHT));
            bg.setFillColor(Color(20, 20, 30)); 
            window.draw(bg);

            titleText.setString("TETRIS GAME");
            titleText.setCharacterSize(60);
            titleText.setFillColor(Color::Red);
            titleText.setPosition(0, 60);
            centerText(titleText, 0, TOTAL_WIDTH);
            window.draw(titleText);

            normalText.setString("1. Start Game");
            normalText.setCharacterSize(30); 
            normalText.setFillColor(Color::Cyan);
            normalText.setPosition(0, 220); 
            centerText(normalText, 0, TOTAL_WIDTH);
            window.draw(normalText);

            normalText.setString("2. High Scores");
            normalText.setCharacterSize(30);
            normalText.setPosition(0, 280); 
            centerText(normalText, 0, TOTAL_WIDTH);
            window.draw(normalText);

            normalText.setString("3. Help");
            normalText.setCharacterSize(30);
            normalText.setPosition(0, 340); 
            centerText(normalText, 0, TOTAL_WIDTH);
            window.draw(normalText);

            normalText.setString("4. Exit");
            normalText.setCharacterSize(30); 
            normalText.setPosition(0, 400);
            centerText(normalText, 0, TOTAL_WIDTH);
            window.draw(normalText);
        }
        else if (currentState == PLAY || currentState == PAUSE) {
            RectangleShape sidebar(Vector2f(SIDEBAR_WIDTH, HEIGHT));
            sidebar.setPosition(WIDTH, 0);
            sidebar.setFillColor(Color(40, 40, 40));
            sidebar.setOutlineThickness(2);
            sidebar.setOutlineColor(Color::White);
            window.draw(sidebar);

            if (showText) {
                titleText.setString("TETRIS");
                titleText.setCharacterSize(45);

                titleText.setFillColor(Color::Black);
                titleText.setPosition(WIDTH + 4, 64);
                centerText(titleText, WIDTH, SIDEBAR_WIDTH);
                window.draw(titleText);

                titleText.setFillColor(Color::Magenta); 
                titleText.setPosition(WIDTH, 60);       
                centerText(titleText, WIDTH, SIDEBAR_WIDTH);
                window.draw(titleText);

                normalText.setCharacterSize(22);
                normalText.setString("Score");
                normalText.setFillColor(Color::Cyan);
                normalText.setPosition(WIDTH, 130); 
                centerText(normalText, WIDTH, SIDEBAR_WIDTH);
                window.draw(normalText);

                normalText.setString(to_string(score));
                normalText.setFillColor(Color::White);
                normalText.setPosition(WIDTH, 155);
                centerText(normalText, WIDTH, SIDEBAR_WIDTH);
                window.draw(normalText);

                normalText.setString("Level");
                normalText.setFillColor(Color::Cyan);
                normalText.setPosition(WIDTH, 190);
                centerText(normalText, WIDTH, SIDEBAR_WIDTH);
                window.draw(normalText);

                normalText.setString(to_string(level));
                normalText.setFillColor(Color::White);
                normalText.setPosition(WIDTH, 215);
                centerText(normalText, WIDTH, SIDEBAR_WIDTH);
                window.draw(normalText);

                normalText.setString("Lines");
                normalText.setFillColor(Color::Cyan);
                normalText.setPosition(WIDTH, 250);
                centerText(normalText, WIDTH, SIDEBAR_WIDTH);
                window.draw(normalText);

                normalText.setString(to_string(linesClearedTotal));
                normalText.setFillColor(Color::White);
                normalText.setPosition(WIDTH, 275);
                centerText(normalText, WIDTH, SIDEBAR_WIDTH);
                window.draw(normalText);

                normalText.setString("NEXT:");
                normalText.setFillColor(Color::Yellow);
                normalText.setPosition(WIDTH, 350);
                centerText(normalText, WIDTH, SIDEBAR_WIDTH);
                window.draw(normalText);

                for (int i = 0; i < 4; i++) {
                    RectangleShape nextBlock(Vector2f(TILE_SIZE - 1.f, TILE_SIZE - 1.f));
                    int nx = figures[nextFigureIdx][i] % 2;
                    int ny = figures[nextFigureIdx][i] / 2;
                    nextBlock.setPosition((float)(WIDTH + 90 + nx * TILE_SIZE), (float)(390 + ny * TILE_SIZE));

                    Color c;
                    switch (nextColorNum) {
                    case 1: c = Color::Cyan; break;
                    case 2: c = Color::Blue; break;
                    case 3: c = Color(255, 165, 0); break;
                    case 4: c = Color::Yellow; break;
                    case 5: c = Color::Green; break;
                    case 6: c = Color::Red; break;
                    case 7: c = Color::Magenta; break;
                    }
                    nextBlock.setFillColor(c);
                    window.draw(nextBlock);
                }
            }

            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    if (field[i][j] == 0) continue;
                    RectangleShape block(Vector2f(TILE_SIZE - 1.f, TILE_SIZE - 1.f));
                    block.setPosition((float)(j * TILE_SIZE), (float)(i * TILE_SIZE));
                    Color c;
                    switch (field[i][j]) {
                    case 1: c = Color::Cyan; break;
                    case 2: c = Color::Blue; break;
                    case 3: c = Color(255, 165, 0); break;
                    case 4: c = Color::Yellow; break;
                    case 5: c = Color::Green; break;
                    case 6: c = Color::Red; break;
                    case 7: c = Color::Magenta; break;
                    default: c = Color::White;
                    }
                    block.setFillColor(c);
                    window.draw(block);
                }
            }

            if (playableRows < M) {
                RectangleShape deadZone(Vector2f((float)WIDTH, (float)((M - playableRows) * TILE_SIZE)));
                deadZone.setPosition(0, (float)(playableRows * TILE_SIZE));
                deadZone.setFillColor(Color(50, 0, 0, 150));
                deadZone.setOutlineColor(Color::Red);
                deadZone.setOutlineThickness(1);
                window.draw(deadZone);
            }

            for (int i = 0; i < 4; i++) {
                RectangleShape sBlock(Vector2f(TILE_SIZE - 1.f, TILE_SIZE - 1.f));
                sBlock.setPosition((float)(shadow[i].x * TILE_SIZE), (float)(shadow[i].y * TILE_SIZE));
                sBlock.setFillColor(Color(100, 100, 100, 80));
                sBlock.setOutlineThickness(1);
                sBlock.setOutlineColor(Color(200, 200, 200, 100));
                window.draw(sBlock);
            }
            for (int i = 0; i < 4; i++) {
                RectangleShape block(Vector2f(TILE_SIZE - 1.f, TILE_SIZE - 1.f));
                block.setPosition((float)(a[i].x * TILE_SIZE), (float)(a[i].y * TILE_SIZE));
                Color c;
                switch (colorNum) {
                case 1: c = Color::Cyan; break;
                case 2: c = Color::Blue; break;
                case 3: c = Color(255, 165, 0); break;
                case 4: c = Color::Yellow; break;
                case 5: c = Color::Green; break;
                case 6: c = Color::Red; break;
                case 7: c = Color::Magenta; break;
                }
                block.setFillColor(c);
                window.draw(block);
            }

            if (currentState == PAUSE && showText) {
                RectangleShape overlay(Vector2f(TOTAL_WIDTH, HEIGHT));
                overlay.setFillColor(Color(0, 0, 0, 240)); 
                window.draw(overlay);

                titleText.setString("PAUSED");
                titleText.setFillColor(Color::White);
                titleText.setPosition(0, 100);
                centerText(titleText, 0, TOTAL_WIDTH);
                window.draw(titleText);

                normalText.setCharacterSize(25); 
                normalText.setFillColor(Color::Green); 
                normalText.setString("1. Continue");
                normalText.setPosition(0, 200);
                centerText(normalText, 0, TOTAL_WIDTH);
                window.draw(normalText);

                normalText.setFillColor(Color::Red); 
                normalText.setString("2. Press Enter to Restart");
                normalText.setPosition(0, 250);
                centerText(normalText, 0, TOTAL_WIDTH);
                window.draw(normalText);

                normalText.setFillColor(Color::Yellow); 
                normalText.setString("3. High Scores");
                normalText.setPosition(0, 300);
                centerText(normalText, 0, TOTAL_WIDTH);
                window.draw(normalText);

                normalText.setFillColor(Color::Cyan);
                normalText.setString("4. Exit");
                normalText.setPosition(0, 350);
                centerText(normalText, 0, TOTAL_WIDTH);
                window.draw(normalText);
            }
        }
        else if (showText && currentState == HIGHSCORE) {
            RectangleShape bg(Vector2f(TOTAL_WIDTH, HEIGHT));
            bg.setFillColor(Color(20, 20, 30));
            window.draw(bg);

            vector<int> scores = loadHighScores();
            string s = "HIGH SCORES:\n\n";
            for (size_t i = 0; i < scores.size(); i++) s += to_string(i + 1) + ". " + to_string(scores[i]) + "\n";
            s += "\n(Press ESC to return)";

            normalText.setCharacterSize(25);
            normalText.setString(s);
            normalText.setPosition(50, 50);
            window.draw(normalText);
        }
        else if (showText && currentState == HELP) {
            RectangleShape bg(Vector2f(TOTAL_WIDTH, HEIGHT));
            bg.setFillColor(Color(20, 20, 30));
            window.draw(bg);

            titleText.setString("CONTROLS");
            titleText.setPosition(180, 100);
            window.draw(titleText);

            normalText.setCharacterSize(22);
            normalText.setString("\n\nMove: Arrows / WASD\n\nHard Drop: Spacebar\n\nPause: P\n\nEsc: Back to Menu");
            normalText.setPosition(50, 100);
            window.draw(normalText);
        }
        else if (showText && currentState == GAMEOVER) {
            RectangleShape overlay(Vector2f(TOTAL_WIDTH, HEIGHT));
            overlay.setFillColor(Color(0, 0, 0, 200));
            window.draw(overlay);

            titleText.setString("GAME OVER");
            titleText.setFillColor(Color::Red);
            titleText.setPosition(0, 200);
            centerText(titleText, 0, TOTAL_WIDTH);
            window.draw(titleText);

            normalText.setString("Final Score: " + to_string(score) + "\n\nPress ESC for Menu");
            normalText.setPosition(0, 300);
            centerText(normalText, 0, TOTAL_WIDTH);
            window.draw(normalText);
        }

        window.display();
    }
    return 0;
}