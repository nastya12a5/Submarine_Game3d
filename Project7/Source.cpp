#include <GL/freeglut.h>
#include <vector>
#include <ctime>
#include <cmath>
#include <string>
#include <sstream>

int windowWidth = 800, windowHeight = 600;
bool gameOver = false;
int score = 0;
float bestTime = 0.0f, currentTime = 0.0f;
clock_t startTime;
bool gameStarted = true;

// Структура подлодки
struct Submarine {
    float x = 0.0f, y = 0.0f, z = -5.0f;
    float speed = 0.1f;
    float rx = 0.3f, ry = 0.15f, rz = 0.15f;
} submarine;

// Монетки
struct Coin {
    float x, y, z;
    float radius = 0.1f;
    bool collected = false;
};
std::vector<Coin> coins;

// Структура для световых лучей
struct LightRay {
    float x, y, z;
    float speed;
    float length;
};

std::vector<LightRay> lightRays;

std::string formatTime(float seconds) {
    int mins = int(seconds) / 60;
    int secs = int(seconds) % 60;
    int msec = int(seconds * 1000) % 1000;
    char buf[20];
    sprintf_s(buf, "%02d:%02d.%03d", mins, secs, msec);
    return std::string(buf);
}

float randFloat(float a, float b) {
    return a + static_cast<float>(rand()) / RAND_MAX * (b - a);
}

void initGame() {
    coins.clear();
    lightRays.clear();
    score = 0;
    gameOver = false;
    submarine = {};

    // Создаем 10 монеток в случайных позициях
    for (int i = 0; i < 10; i++) {
        coins.push_back({ randFloat(-1.7, 1.7), randFloat(-1.7, 1.7), randFloat(-7.5, -2.5), 0.1f });
    }

    // Создаем световые лучи для фона
    for (int i = 0; i < 20; i++) {
        lightRays.push_back({
            randFloat(-3.0f, 3.0f),
            randFloat(-3.0f, 3.0f),
            randFloat(-10.0f, 0.0f),
            randFloat(0.01f, 0.05f),
            randFloat(0.5f, 2.0f)
            });
    }

    startTime = clock();
    gameStarted = true;
}

void drawSubmarine() {
    glPushMatrix();
    glTranslatef(submarine.x, submarine.y, submarine.z);

    // Корпус подлодки
    glColor3f(0.3f, 0.3f, 0.8f);
    GLUquadric* quad = gluNewQuadric();
    glScalef(submarine.rx, submarine.ry, submarine.rz);
    gluSphere(quad, 1.0f, 32, 32);
    gluDeleteQuadric(quad);

    // Кабина
    glColor3f(0.5f, 0.5f, 1.0f);
    glTranslatef(0.0f, 0.2f, 0.0f);
    glutSolidSphere(0.3f, 16, 16);

    glPopMatrix();

    // Тень от подлодки
    glPushMatrix();
    glTranslatef(submarine.x, -1.5f, submarine.z);
    glScalef(1.0f, 0.1f, 1.0f);
    glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
    glutSolidSphere(submarine.rx * 1.2f, 16, 16);
    glPopMatrix();
}

void drawCoin(const Coin& coin) {
    if (coin.collected) return;
    glPushMatrix();
    glTranslatef(coin.x, coin.y, coin.z);

    // Внешний круг
    glColor3f(1.0f, 0.85f, 0.0f);
    GLUquadric* quad = gluNewQuadric();
    gluDisk(quad, 0, coin.radius, 32, 1);

    // Внутренний круг
    glColor3f(1.0f, 1.0f, 0.5f);
    gluDisk(quad, 0, coin.radius * 0.7f, 32, 1);
    gluDeleteQuadric(quad);

    glPopMatrix();
}

void drawLightRays() {
    glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
    glBegin(GL_LINES);
    for (auto& ray : lightRays) {
        glVertex3f(ray.x, ray.y, ray.z);
        glVertex3f(ray.x, ray.y + ray.length, ray.z);
    }
    glEnd();
}

void updateLightRays() {
    for (auto& ray : lightRays) {
        ray.y += ray.speed;
        if (ray.y > 3.0f) {
            ray.y = -3.0f;
            ray.x = randFloat(-3.0f, 3.0f);
            ray.z = randFloat(-10.0f, 0.0f);
        }
    }
}

void drawText(float x, float y, const std::string& text) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRasterPos2f(x, y);
    for (char c : text) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void checkCollisions() {
    for (auto& coin : coins) {
        if (coin.collected) continue;
        float dx = coin.x - submarine.x;
        float dy= coin.y - submarine.y;
        float dz = coin.z - submarine.z;
        float dist = sqrt(dx * dx + dy * dy + dz * dz);
        if (dist < submarine.rx + coin.radius) {
            coin.collected = true;
            score++;
        }
    }
    gameOver = true;
    for (const auto& coin : coins)
        if (!coin.collected) { gameOver = false; break; }

    if (gameOver) {
        float timeSpent = float(clock() - startTime) / CLOCKS_PER_SEC;
        if (bestTime == 0.0f || timeSpent < bestTime)
            bestTime = timeSpent;
    }
}

void drawBackground() {
    // Градиентный фон (от темно-синего вверху к синему внизу)
    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.1f, 0.3f); // Верхний цвет
    glVertex3f(-5.0f, -5.0f, -10.0f);
    glVertex3f(5.0f, -5.0f, -10.0f);
    glColor3f(0.0f, 0.3f, 0.6f); // Нижний цвет
    glVertex3f(5.0f, 5.0f, -10.0f);
    glVertex3f(-5.0f, 5.0f, -10.0f);
    glEnd();
    // Боковые стены - темно-синие с текстурой
    glColor3f(0.05f, 0.1f, 0.15f);
    glBegin(GL_QUADS);
    // Левая стена
    glVertex3f(-5.0f, -2.0f, -10.0f);
    glVertex3f(-5.0f, -2.0f, 0.0f);
    glVertex3f(-5.0f, 5.0f, 0.0f);
    glVertex3f(-5.0f, 5.0f, -10.0f);
    // Правая стена
    glVertex3f(5.0f, -2.0f, -10.0f);
    glVertex3f(5.0f, 5.0f, -10.0f);
    glVertex3f(5.0f, 5.0f, 0.0f);
    glVertex3f(5.0f, -2.0f, 0.0f);
    glEnd();
    // "Дно"
    glColor3f(0.35f, 0.3f, 0.2f);
    glBegin(GL_QUADS);
    glVertex3f(-5.0f, -2.0f, -10.0f);
    glVertex3f(5.0f, -2.0f, -10.0f);
    glVertex3f(5.0f, -2.0f, 0.0f);
    glVertex3f(-5.0f, -2.0f, 0.0f);
    glEnd();
    // Добавим немного текстурности дну
    glColor3f(0.4f, 0.35f, 0.25f);  // Более светлый оттенок для деталей
    glBegin(GL_POINTS);
    for (int i = 0; i < 500; i++) {
        glVertex3f(randFloat(-5.0f, 5.0f), -2.0f, randFloat(-10.0f, 0.0f));
    }
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Устанавливаем камеру немного выше подлодки
    gluLookAt(0, 1.0f, 0, 0, 0, -5, 0, 1, 0);

    // Рисуем фон
    drawBackground();

    // Обновляем и рисуем световые лучи
    updateLightRays();
    drawLightRays();

    // Монетки
    for (auto& c : coins) drawCoin(c);

    // Подлодка
    drawSubmarine();

    // HUD
    std::stringstream ss;
    ss << "Score: " << score;
    glColor3f(1, 1, 1);
    drawText(10, windowHeight - 30, ss.str());

    if (gameStarted && !gameOver) {
        currentTime = float(clock() - startTime) / CLOCKS_PER_SEC;
        ss.str(""); ss << "Time: " << formatTime(currentTime);
        drawText(10, windowHeight - 60, ss.str());
    }

    if (bestTime > 0.0f) {
        ss.str(""); ss<< "Best: " <<formatTime(bestTime);
        drawText(10, windowHeight - 90, ss.str());
    }

    if (gameOver) {
        glColor3f(1.0f, 1.0f, 0.0f);
        drawText(windowWidth / 2 - 100, windowHeight / 2, "GAME OVER! Press R to restart");
    }

    glutSwapBuffers();
}

void reshape(int w, int h) {
    windowWidth = w; windowHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, double(w) / h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int, int) {
    switch (key) {
    case 'w': submarine.y += submarine.speed; break;
    case 's': submarine.y -= submarine.speed; break;
    case 'a': submarine.x -= submarine.speed; break;
    case 'd': submarine.x += submarine.speed; break;
    case 'q': submarine.z += submarine.speed; break;
    case 'e': submarine.z -= submarine.speed; break;
    case 'r': if (gameOver) initGame(); break;
    case '+': submarine.speed += 0.01f; break; // Увеличение скорости
    case '-': submarine.speed = fmax(0.01f, submarine.speed - 0.01f); break; // Уменьшение скорости
    }
    checkCollisions();
    glutPostRedisplay();
}

void timer(int) {
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("3D Submarine Adventure");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    srand(static_cast<unsigned>(time(0)));
    initGame();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}



