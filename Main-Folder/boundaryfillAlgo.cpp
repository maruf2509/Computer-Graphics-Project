#include <GL/glut.h>
#include <iostream>
#include <stack>
#include <cmath>
using namespace std;

const int WIDTH = 640;
const int HEIGHT = 480;

const unsigned char FILL_COLOR[] = {255, 0, 0};
const unsigned char BOUNDARY_COLOR[] = {255, 255, 255};
const unsigned char BG_COLOR[] = {0, 0, 0};

int centerX = WIDTH / 2, centerY = HEIGHT / 2;
int radius = 100;

unsigned char pixelBuffer[HEIGHT][WIDTH][3];
stack<pair<int, int>> fillPixels;
bool fillInitialized = false;

void setPixel(int x, int y, const unsigned char color[3]) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        pixelBuffer[y][x][0] = color[0];
        pixelBuffer[y][x][1] = color[1];
        pixelBuffer[y][x][2] = color[2];
    }
}

const unsigned char* getPixel(int x, int y) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        return pixelBuffer[y][x];
    }
    return nullptr;
}

void drawCircleToBuffer() {
    // ব্যাকগ্রাউন্ড আগেই সেট করা আছে init() তে, তাই এখানে শুধু বর্ডার আঁকবো
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    while (y >= x) {
        setPixel(centerX + x, centerY + y, BOUNDARY_COLOR);
        setPixel(centerX - x, centerY + y, BOUNDARY_COLOR);
        setPixel(centerX + x, centerY - y, BOUNDARY_COLOR);
        setPixel(centerX - x, centerY - y, BOUNDARY_COLOR);
        setPixel(centerX + y, centerY + x, BOUNDARY_COLOR);
        setPixel(centerX - y, centerY + x, BOUNDARY_COLOR);
        setPixel(centerX + y, centerY - x, BOUNDARY_COLOR);
        setPixel(centerX - y, centerY - x, BOUNDARY_COLOR);

        x++;
        if (d > 0) {
            y--;
            d += 4 * (x - y) + 10;
        } else {
            d += 4 * x + 6;
        }
    }
}

void boundaryFillStep() {
    const int PIXELS_PER_FRAME = 1000;

    for (int i = 0; i < PIXELS_PER_FRAME && !fillPixels.empty(); ++i) {
        pair<int, int> current = fillPixels.top();
        fillPixels.pop();

        int x = current.first;
        int y = current.second;

        const unsigned char* pixel = getPixel(x, y);
        if (!pixel) continue;

        bool isBoundary = (pixel[0] == BOUNDARY_COLOR[0] &&
                           pixel[1] == BOUNDARY_COLOR[1] &&
                           pixel[2] == BOUNDARY_COLOR[2]);

        bool isFilled = (pixel[0] == FILL_COLOR[0] &&
                         pixel[1] == FILL_COLOR[1] &&
                         pixel[2] == FILL_COLOR[2]);

        if (!isBoundary && !isFilled) {
            setPixel(x, y, FILL_COLOR);
            fillPixels.push({x - 1, y});
            fillPixels.push({x + 1, y});
            fillPixels.push({x, y - 1});
            fillPixels.push({x, y + 1});
        }
    }

    if (!fillPixels.empty()) {
        glutPostRedisplay();
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawCircleToBuffer();

    glBegin(GL_POINTS);
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            glColor3ub(pixelBuffer[y][x][0], pixelBuffer[y][x][1], pixelBuffer[y][x][2]);
            glVertex2i(x, y);
        }
    }
    glEnd();

    glFlush();

    if (fillInitialized)
        boundaryFillStep();
}

void mouse(int btn, int state, int x, int y) {
    if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        int clickX = x;
        int clickY = HEIGHT - y; // OpenGL coordinate system adjustment

        // Distance from circle center
        int dx = clickX - centerX;
        int dy = clickY - centerY;
        double dist = sqrt(dx * dx + dy * dy);

        if (dist < radius) {  // ক্লিক সার্কেলের ভিতর হলে fill শুরু করো
            fillInitialized = true;

            while (!fillPixels.empty()) fillPixels.pop();

            fillPixels.push({clickX, clickY});

            glutPostRedisplay();
        } else {
            // সার্কেলের বাইরে ক্লিক করলে fill হবেনা
            cout << "Click outside the circle. No fill started.\n";
        }
    }
}

void init() {
    glClearColor(BG_COLOR[0] / 255.0f, BG_COLOR[1] / 255.0f, BG_COLOR[2] / 255.0f, 1.0);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
    glPointSize(1.0);

    // ব্যাকগ্রাউন্ড কালার একবারেই সেট করো
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            setPixel(x, y, BG_COLOR);
        }
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Boundary Fill Circle");

    init();

    glutDisplayFunc(display);
    glutMouseFunc(mouse);

    glutMainLoop();

    return 0;
}
