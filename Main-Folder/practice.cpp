#include<windows.h>
#include<GL/glut.h>
#include<stdlib.h>
#include<stdio.h>
#include<math.h>

void drawPixel(float x, float y) {
glBegin(GL_POINTS);
glVertex2f(x, y);
glEnd();
}

void bresenhamLine(float x1, float y1, float x2, float y2) {
float dx = fabs(x2 - x1);
float dy = fabs(y2 - y1);
float x = x1;
float y = y1;

int xInc = (x2 > x1) ? 1 : -1;
int yInc = (y2 > y1) ? 1 : -1;

drawPixel(x, y);

if (dy <= dx) {
// 0 < m < 1
float p = 2 * dy - dx;
for (int i = 0; i < dx; i++) {
x += xInc;
if (p < 0) {
p += 2 * dy;
} else {
y += yInc;
p += 2 * (dy - dx);
}
drawPixel(x, y);
}
} else {
// m > 1
float p = 2 * dx - dy;
for (int i = 0; i < dy; i++) {
y += yInc;
if (p < 0) {
p += 2 * dx;
} else {
x += xInc;
p += 2 * (dx - dy);
}
drawPixel(x, y);
}
}
}

float x1, ya, x2, y2;

void display(void) {
glClear(GL_COLOR_BUFFER_BIT);

// Rectangle corners
float x3 = x2, y3 = ya;
float x4 = x1, y4 = y2;

// Draw rectangle edges using Bresenham's line algorithm
bresenhamLine(x1, ya, x3, y3); // bottom edge
bresenhamLine(x3, y3, x2, y2); // right edge
bresenhamLine(x2, y2, x4, y4); // top edge
bresenhamLine(x4, y4, x1, ya); // left edge

glFlush();
}

void init(void) {
glClearColor(0, 0, 0, 0);
glMatrixMode(GL_PROJECTION);
glLoadIdentity();
gluOrtho2D(-100, 100, -100, 100);
}

int main(int argc, char** argv) {
printf("Enter two diagonal corner points of rectangle (x1 y1 x2 y2):\n");
scanf("%f %f %f %f", &x1, &ya, &x2, &y2);

glutInit(&argc, argv);
glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
glutInitWindowSize(500, 500);
glutInitWindowPosition(100, 100);
glutCreateWindow("Rectangle using Bresenham Line Algorithm");

init();
glutDisplayFunc(display);
glutMainLoop();

return 0;
}