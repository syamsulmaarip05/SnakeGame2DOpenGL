#include <GL/freeglut.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <cmath>

// Ukuran window
const int window_width = 800;
const int window_height = 600;

// Ukuran grid dan snake
const int grid_size = 20;
const int cols = window_width / grid_size;
const int rows = window_height / grid_size;

// Warna-warna yang akan digunakan
const GLfloat colors[][3] = {
    {0.0f, 0.8f, 0.0f},  // Snake hijau
    {0.0f, 1.0f, 0.0f},  // Snake head hijau terang
    {1.0f, 0.0f, 0.0f},  // Makanan merah
    {1.0f, 0.65f, 0.0f}, // Rintangan orange
    {0.5f, 0.5f, 0.5f},  // Border abu-abu
    {0.2f, 0.2f, 0.9f},  // Power-up biru
};

// Mode permainan
enum GameMode
{
    CLASSIC,
    CHALLENGE
};

// Snake dan makanan
std::vector<std::pair<int, int>> snake = {{10, 10}};
int dir_x = 1, dir_y = 0;
std::pair<int, int> food = {15, 15};
std::pair<int, int> powerUp = {-1, -1}; // Inisialisasi di luar layar

// Level dan skor
int level = 1;
int score = 0;
bool gameOver = false;
bool pauseGame = false;
GameMode gameMode = CLASSIC;

// Timer game
int gameSpeed = 150; // milliseconds
int powerUpCounter = 0;
int powerUpDuration = 0;
bool isPowerUpActive = false;
bool isPowerUpVisible = false;

// Rintangan tembok
std::vector<std::pair<int, int>> walls;

// Efek visual
float pulseValue = 0.0f;
bool pulseDirection = true;

// Fungsi untuk spawn makanan
void spawnFood()
{
    bool valid = false;
    while (!valid)
    {
        // Batasi area spawn untuk menghindari spawn di dekat tepi
        food.first = 1 + (rand() % (cols - 2));
        food.second = 1 + (rand() % (rows - 2));

        // Periksa apakah makanan tidak tumpang tindih dengan tubuh ular
        valid = true;
        for (auto segment : snake)
        {
            if (segment.first == food.first && segment.second == food.second)
            {
                valid = false;
                break;
            }
        }

        // Periksa apakah makanan tidak tumpang tindih dengan tembok
        for (auto wall : walls)
        {
            if (wall.first == food.first && wall.second == food.second)
            {
                valid = false;
                break;
            }
        }

        // Periksa apakah makanan tidak tumpang tindih dengan power-up
        if (powerUp.first == food.first && powerUp.second == food.second)
        {
            valid = false;
        }
    }
}

// Fungsi untuk spawn power-up
void spawnPowerUp()
{
    if (rand() % 100 < 30) // 30% chance untuk power-up muncul
    {
        bool valid = false;
        while (!valid)
        {
            powerUp.first = 1 + (rand() % (cols - 2));
            powerUp.second = 1 + (rand() % (rows - 2));

            // Periksa apakah power-up tidak tumpang tindih dengan tubuh ular
            valid = true;
            for (auto segment : snake)
            {
                if (segment.first == powerUp.first && segment.second == powerUp.second)
                {
                    valid = false;
                    break;
                }
            }

            // Periksa apakah power-up tidak tumpang tindih dengan tembok
            for (auto wall : walls)
            {
                if (wall.first == powerUp.first && wall.second == powerUp.second)
                {
                    valid = false;
                    break;
                }
            }

            // Periksa apakah power-up tidak tumpang tindih dengan makanan
            if (food.first == powerUp.first && food.second == powerUp.second)
            {
                valid = false;
            }
        }
        isPowerUpVisible = true;
    }
    else
    {
        // Tidak spawn power-up
        powerUp.first = -1;
        powerUp.second = -1;
        isPowerUpVisible = false;
    }
}

// Fungsi untuk membuat rintangan tembok
void spawnWalls()
{
    walls.clear();
    int wallCount = 5 + (level - 1) * 2; // Makin tinggi level, makin banyak tembok
    if (wallCount > 20)
        wallCount = 20; // Maksimal 20 tembok

    if (gameMode == CHALLENGE)
    {
        // Tata letak tembok berdasarkan level
        switch (level % 3)
        {
        case 1: // Pola tembok horizontal
            for (int i = 0; i < 3; i++)
            {
                for (int j = 5; j < cols - 5; j++)
                {
                    if (j % 2 == 0)
                        walls.push_back({j, rows / 4 + (i * rows / 4)});
                }
            }
            break;
        case 2: // Pola tembok vertikal
            for (int i = 0; i < 3; i++)
            {
                for (int j = 5; j < rows - 5; j++)
                {
                    if (j % 2 == 0)
                        walls.push_back({cols / 4 + (i * cols / 4), j});
                }
            }
            break;
        case 0: // Pola maze
            for (int i = 0; i < cols; i++)
            {
                if (i % 5 == 0)
                {
                    for (int j = 1; j < rows / 2; j++)
                    {
                        walls.push_back({i, j});
                    }
                }
                if (i % 5 == 2)
                {
                    for (int j = rows / 2; j < rows - 1; j++)
                    {
                        walls.push_back({i, j});
                    }
                }
            }
            break;
        }
    }
    else
    {
        // Mode klasik: tembok acak
        for (int i = 0; i < wallCount; i++)
        {
            int wx = 1 + (rand() % (cols - 2));
            int wy = 1 + (rand() % (rows - 2));

            // Jangan spawn tembok terlalu dekat dengan ular
            bool tooClose = false;
            for (auto segment : snake)
            {
                if (abs(segment.first - wx) < 3 && abs(segment.second - wy) < 3)
                {
                    tooClose = true;
                    break;
                }
            }

            if (!tooClose)
            {
                walls.push_back({wx, wy});
            }
            else
            {
                i--; // Coba lagi
            }
        }
    }
}

// Fungsi untuk menampilkan skor
void displayScore()
{
    std::stringstream ss;
    ss << "Score: " << score << " | Level: " << level;
    if (gameMode == CLASSIC)
    {
        ss << " | Mode: Classic";
    }
    else
    {
        ss << " | Mode: Challenge";
    }

    if (isPowerUpActive)
    {
        ss << " | POWER UP: " << powerUpDuration;
    }

    std::string scoreText = ss.str();

    glColor3f(1.0f, 1.0f, 1.0f);                 // Warna putih untuk teks
    glRasterPos2f(10.0f, window_height - 20.0f); // Posisi teks
    for (char c : scoreText)
    {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }
}

// Fungsi untuk menampilkan Game Over
void displayGameOver()
{
    std::string gameOverText = "Game Over! Press F1 to Restart. Press M to change mode.";
    glColor3f(1.0f, 0.0f, 0.0f); // Warna merah untuk Game Over
    glRasterPos2f(window_width / 2 - 170.0f, window_height / 2);
    for (char c : gameOverText)
    {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }

    std::stringstream ss;
    ss << "Final Score: " << score;
    std::string scoreText = ss.str();
    glRasterPos2f(window_width / 2 - 50.0f, window_height / 2 - 30.0f);
    for (char c : scoreText)
    {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }
}

// Efek visual untuk pulsa
void updatePulseEffect()
{
    if (pulseDirection)
    {
        pulseValue += 0.05f;
        if (pulseValue >= 1.0f)
        {
            pulseValue = 1.0f;
            pulseDirection = false;
        }
    }
    else
    {
        pulseValue -= 0.05f;
        if (pulseValue <= 0.0f)
        {
            pulseValue = 0.0f;
            pulseDirection = true;
        }
    }
}

// Display function
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Gambar Snake
    for (size_t i = 0; i < snake.size(); i++)
    {
        if (i == 0) // Snake head
        {
            if (isPowerUpActive)
            {
                // Efek berkedip saat power-up aktif
                float intensity = 0.5f + 0.5f * pulseValue;
                glColor3f(intensity, intensity, 0.0f); // Warna kuning berkedip
            }
            else
            {
                glColor3fv(colors[1]); // Warna kepala snake
            }
        }
        else
        {
            if (isPowerUpActive)
            {
                // Warna tubuh saat power-up aktif
                float r = 0.5f + 0.3f * sin(i * 0.5f + pulseValue * 3.14f);
                float g = 0.5f + 0.3f * sin(i * 0.5f + pulseValue * 3.14f + 2.0f);
                float b = 0.0f;
                glColor3f(r, g, b);
            }
            else
            {
                // Warna tubuh gradasi
                float intensity = 1.0f - (float)i / snake.size() * 0.5f;
                glColor3f(colors[0][0] * intensity,
                          colors[0][1] * intensity,
                          colors[0][2] * intensity);
            }
        }

        glBegin(GL_QUADS);
        glVertex2f(snake[i].first * grid_size, snake[i].second * grid_size);
        glVertex2f((snake[i].first + 1) * grid_size, snake[i].second * grid_size);
        glVertex2f((snake[i].first + 1) * grid_size, (snake[i].second + 1) * grid_size);
        glVertex2f(snake[i].first * grid_size, (snake[i].second + 1) * grid_size);
        glEnd();
    }

    // Gambar Makanan dengan efek pulsa
    float foodSize = 0.8f + 0.2f * pulseValue;
    float foodOffset = grid_size * (1.0f - foodSize) / 2.0f;

    glColor3fv(colors[2]); // Warna makanan merah
    glBegin(GL_QUADS);
    glVertex2f(food.first * grid_size + foodOffset, food.second * grid_size + foodOffset);
    glVertex2f((food.first + foodSize) * grid_size + foodOffset, food.second * grid_size + foodOffset);
    glVertex2f((food.first + foodSize) * grid_size + foodOffset, (food.second + foodSize) * grid_size + foodOffset);
    glVertex2f(food.first * grid_size + foodOffset, (food.second + foodSize) * grid_size + foodOffset);
    glEnd();

    // Gambar PowerUp
    if (isPowerUpVisible)
    {
        float powerUpSize = 0.6f + 0.4f * pulseValue;
        float powerUpOffset = grid_size * (1.0f - powerUpSize) / 2.0f;

        glColor3fv(colors[5]); // Warna biru
        glBegin(GL_QUADS);
        glVertex2f(powerUp.first * grid_size + powerUpOffset, powerUp.second * grid_size + powerUpOffset);
        glVertex2f((powerUp.first + powerUpSize) * grid_size + powerUpOffset, powerUp.second * grid_size + powerUpOffset);
        glVertex2f((powerUp.first + powerUpSize) * grid_size + powerUpOffset, (powerUp.second + powerUpSize) * grid_size + powerUpOffset);
        glVertex2f(powerUp.first * grid_size + powerUpOffset, (powerUp.second + powerUpSize) * grid_size + powerUpOffset);
        glEnd();
    }

    // Gambar Tembok (Rintangan)
    glColor3fv(colors[3]); // Warna orange
    for (auto wall : walls)
    {
        glBegin(GL_QUADS);
        glVertex2f(wall.first * grid_size, wall.second * grid_size);
        glVertex2f((wall.first + 1) * grid_size, wall.second * grid_size);
        glVertex2f((wall.first + 1) * grid_size, (wall.second + 1) * grid_size);
        glVertex2f(wall.first * grid_size, (wall.second + 1) * grid_size);
        glEnd();
    }

    // Gambar Tembok Border
    glColor3fv(colors[4]); // Warna abu-abu untuk border
    for (int i = 0; i < cols; i++)
    {
        glBegin(GL_QUADS);
        glVertex2f(i * grid_size, 0); // Baris bawah
        glVertex2f((i + 1) * grid_size, 0);
        glVertex2f((i + 1) * grid_size, grid_size);
        glVertex2f(i * grid_size, grid_size);
        glEnd();

        glBegin(GL_QUADS);
        glVertex2f(i * grid_size, window_height - grid_size); // Baris atas
        glVertex2f((i + 1) * grid_size, window_height - grid_size);
        glVertex2f((i + 1) * grid_size, window_height);
        glVertex2f(i * grid_size, window_height);
        glEnd();
    }

    for (int i = 0; i < rows; i++)
    {
        glBegin(GL_QUADS);
        glVertex2f(0, i * grid_size); // Kolom kiri
        glVertex2f(grid_size, i * grid_size);
        glVertex2f(grid_size, (i + 1) * grid_size);
        glVertex2f(0, (i + 1) * grid_size);
        glEnd();

        glBegin(GL_QUADS);
        glVertex2f(window_width - grid_size, i * grid_size); // Kolom kanan
        glVertex2f(window_width, i * grid_size);
        glVertex2f(window_width, (i + 1) * grid_size);
        glVertex2f(window_width - grid_size, (i + 1) * grid_size);
        glEnd();
    }

    // Tampilkan skor
    displayScore();

    // Tampilkan Game Over jika game selesai
    if (gameOver)
    {
        displayGameOver();
    }

    // Tampilkan pesan pause jika game di-pause
    if (pauseGame && !gameOver)
    {
        std::string pauseText = "GAME PAUSED - Press P to Resume";
        glColor3f(1.0f, 1.0f, 0.0f); // Warna kuning untuk pause
        glRasterPos2f(window_width / 2 - 120.0f, window_height / 2);
        for (char c : pauseText)
        {
            glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
        }
    }

    glutSwapBuffers();
}

// Keyboard input untuk menggerakkan ular
void specialKeyboard(int key, int x, int y)
{
    if (gameOver)
    {
        if (key == GLUT_KEY_F1) // Menekan F1 untuk restart
        {
            gameOver = false;
            level = 1;
            score = 0;
            snake = {{10, 10}};
            dir_x = 1;
            dir_y = 0;
            gameSpeed = 150;
            isPowerUpActive = false;
            isPowerUpVisible = false;
            spawnFood();
            spawnWalls();
            spawnPowerUp();
            glutPostRedisplay();
        }
    }
    else if (!pauseGame)
    {
        switch (key)
        {
        case GLUT_KEY_UP:
            if (dir_y != -1) // Pastikan tidak bergerak ke arah berlawanan
            {
                dir_x = 0;
                dir_y = 1; // Gerakan ke atas (nilai Y meningkat)
            }
            break;
        case GLUT_KEY_DOWN:
            if (dir_y != 1) // Pastikan tidak bergerak ke arah berlawanan
            {
                dir_x = 0;
                dir_y = -1; // Gerakan ke bawah (nilai Y menurun)
            }
            break;
        case GLUT_KEY_LEFT:
            if (dir_x != 1) // Pastikan tidak bergerak ke arah berlawanan
            {
                dir_x = -1;
                dir_y = 0;
            }
            break;
        case GLUT_KEY_RIGHT:
            if (dir_x != -1) // Pastikan tidak bergerak ke arah berlawanan
            {
                dir_x = 1;
                dir_y = 0;
            }
            break;
        }
    }
}

// Keyboard normal untuk kontrol lain
void normalKeyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'p':
    case 'P': // Tombol pause
        pauseGame = !pauseGame;
        break;
    case 'm':
    case 'M': // Tombol ganti mode (saat game over)
        if (gameOver)
        {
            gameMode = (gameMode == CLASSIC) ? CHALLENGE : CLASSIC;
        }
        break;
    case 27: // ESC key
        exit(0);
        break;
    }
    glutPostRedisplay();
}

// Update posisi Snake
void update(int value)
{
    updatePulseEffect(); // Update efek visual pulsa

    if (!gameOver && !pauseGame)
    {
        // Pindahkan snake
        int new_x = snake[0].first + dir_x;
        int new_y = snake[0].second + dir_y;

        // Cek collision dengan tembok
        bool hitWall = false;
        for (auto wall : walls)
        {
            if (wall.first == new_x && wall.second == new_y)
            {
                if (!isPowerUpActive)
                { // Jika tidak dalam mode power-up
                    gameOver = true;
                }
                hitWall = true;
                break;
            }
        }

        // Cek collision dengan tembok border
        if (new_x <= 0 || new_y <= 0 || new_x >= cols - 1 || new_y >= rows - 1)
        {
            gameOver = true;
        }

        // Cek collision dengan diri sendiri
        for (size_t i = 1; i < snake.size(); i++) // Skip head itself
        {
            if (snake[i].first == new_x && snake[i].second == new_y)
            {
                if (!isPowerUpActive)
                { // Jika tidak dalam mode power-up
                    gameOver = true;
                }
                break;
            }
        }

        if (!hitWall || isPowerUpActive)
        {
            // Tambahkan kepala baru
            snake.insert(snake.begin(), {new_x, new_y});

            // Cek makan makanan
            if (new_x == food.first && new_y == food.second)
            {
                spawnFood();
                score += 10 * level; // Skor berdasarkan level

                // Level naik setiap 5 makanan
                if ((score / (10 * level)) % 5 == 0)
                {
                    level++;
                    gameSpeed = std::max(50, 150 - (level - 1) * 10); // Makin cepat
                    spawnWalls();
                }

                // Spawn power-up setelah beberapa skor
                powerUpCounter++;
                if (powerUpCounter >= 3)
                {
                    spawnPowerUp();
                    powerUpCounter = 0;
                }
            }
            else
            {
                // Kalau tidak makan makanan, buang ekor
                snake.pop_back();
            }

            // Cek ambil power-up
            if (isPowerUpVisible && new_x == powerUp.first && new_y == powerUp.second)
            {
                isPowerUpVisible = false;
                isPowerUpActive = true;
                powerUpDuration = 10; // 10 langkah durasi power-up
                powerUp = {-1, -1};   // Pindahkan power-up keluar layar
                score += 25;          // Bonus skor
            }

            // Kurangi durasi power-up jika aktif
            if (isPowerUpActive)
            {
                powerUpDuration--;
                if (powerUpDuration <= 0)
                {
                    isPowerUpActive = false;
                }
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(gameSpeed, update, 0);
}

// Setup OpenGL
void init()
{
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f); // Background biru gelap
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, window_width, 0, window_height); // 2D orthographic projection
}

// Tampilkan menu utama
void displayMenu()
{
    std::string menuText = "Snake Game - OpenGL";
    std::string instructionText = "Press F1 to Start";
    std::string modeText = "Press M to change game mode: ";
    std::string currentMode = (gameMode == CLASSIC) ? "Classic" : "Challenge";
    std::string controlText = "Controls: Arrow Keys, P to Pause, ESC to Exit";

    glColor3f(0.0f, 1.0f, 0.0f); // Warna hijau untuk judul
    glRasterPos2f(window_width / 2 - 80.0f, window_height / 2 + 50.0f);
    for (char c : menuText)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    glColor3f(1.0f, 1.0f, 1.0f); // Warna putih untuk instruksi
    glRasterPos2f(window_width / 2 - 60.0f, window_height / 2);
    for (char c : instructionText)
    {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }

    glRasterPos2f(window_width / 2 - 120.0f, window_height / 2 - 30.0f);
    for (char c : modeText)
    {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }

    glColor3f(1.0f, 1.0f, 0.0f); // Warna kuning untuk mode
    glRasterPos2f(window_width / 2 + 100.0f, window_height / 2 - 30.0f);
    for (char c : currentMode)
    {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }

    glColor3f(1.0f, 1.0f, 1.0f); // Warna putih untuk kontrol
    glRasterPos2f(window_width / 2 - 150.0f, window_height / 2 - 60.0f);
    for (char c : controlText)
    {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }
}

int main(int argc, char **argv)
{
    srand(time(0)); // Seed random
    spawnFood();
    spawnWalls();
    spawnPowerUp();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Snake Game 2D - OpenGL");

    glutDisplayFunc(display);
    glutSpecialFunc(specialKeyboard); // Keyboard arrow
    glutKeyboardFunc(normalKeyboard); // Normal keyboard
    glutTimerFunc(gameSpeed, update, 0);

    init();
    glutMainLoop();
    return 0;
}