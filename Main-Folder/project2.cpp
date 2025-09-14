#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>

// Constants
const float PI = 3.14159265359f;
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;
const int CIRCLE_SEGMENTS = 100;

// Global variables
float cameraDistance = 50.0f;
float cameraAngleX = 0.0f;
float cameraAngleY = 0.0f;
bool mousePressed = false;
int lastMouseX, lastMouseY;
bool showOrbits = true;
bool isPaused = false;
float timeSpeed = 1.0f;
float globalTime = 0.0f;
int selectedPlanet = -1;

// Planet structure
struct Planet {
    std::string name;
    float radius;
    float distance;
    float speed;
    float angle;
    float r, g, b;
    std::vector<Planet> moons;
    
    Planet(std::string n, float rad, float dist, float spd, float red, float green, float blue)
        : name(n), radius(rad), distance(dist), speed(spd), angle(0), r(red), g(green), b(blue) {}
};

// Solar system data
std::vector<Planet> planets;
Planet sun("Sun", 3.0f, 0.0f, 0.0f, 1.0f, 0.8f, 0.0f);

// Function prototypes
void initOpenGL();
void setupSolarSystem();
void display();
void reshape(int width, int height);
void keyboard(unsigned char key, int x, int y);
void specialKeys(int key, int x, int y);
void mouse(int button, int state, int x, int y);
void mouseMotion(int x, int y);
void update(int value);
void drawCircle(float radius, int segments = CIRCLE_SEGMENTS);
void drawOrbit(float radius);
void drawPlanet(const Planet& planet, float x, float y, float z);
void drawText(float x, float y, const std::string& text);
void drawStars();

// Bresenham's circle algorithm implementation
void plotCirclePoints(int cx, int cy, int x, int y) {
    glVertex2f(cx + x, cy + y);
    glVertex2f(cx - x, cy + y);
    glVertex2f(cx + x, cy - y);
    glVertex2f(cx - x, cy - y);
    glVertex2f(cx + y, cy + x);
    glVertex2f(cx - y, cy + x);
    glVertex2f(cx + y, cy - x);
    glVertex2f(cx - y, cy - x);
}

void bresenhamCircle(int cx, int cy, int r) {
    int x = 0, y = r;
    int d = 3 - 2 * r;
    
    glBegin(GL_POINTS);
    plotCirclePoints(cx, cy, x, y);
    
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        plotCirclePoints(cx, cy, x, y);
    }
    glEnd();
}

void initOpenGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    
    // Set up lighting
    GLfloat light_position[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat light_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
    
    setupSolarSystem();
}

void setupSolarSystem() {
    planets.clear();
    
    // Create planets with realistic-ish properties (scaled for visualization)
    planets.push_back(Planet("Mercury", 0.4f, 8.0f, 4.74f, 0.7f, 0.7f, 0.7f));
    planets.push_back(Planet("Venus", 0.9f, 11.0f, 3.50f, 1.0f, 0.8f, 0.0f));
    
    // Earth with Moon
    Planet earth("Earth", 1.0f, 15.0f, 2.98f, 0.2f, 0.6f, 1.0f);
    earth.moons.push_back(Planet("Moon", 0.25f, 2.5f, 13.2f, 0.8f, 0.8f, 0.8f));
    planets.push_back(earth);
    
    planets.push_back(Planet("Mars", 0.5f, 20.0f, 2.41f, 1.0f, 0.4f, 0.2f));
    
    // Jupiter with moons
    Planet jupiter("Jupiter", 2.5f, 30.0f, 1.31f, 1.0f, 0.6f, 0.2f);
    jupiter.moons.push_back(Planet("Io", 0.15f, 4.0f, 17.3f, 1.0f, 1.0f, 0.0f));
    jupiter.moons.push_back(Planet("Europa", 0.13f, 5.0f, 13.7f, 0.8f, 0.9f, 1.0f));
    planets.push_back(jupiter);
    
    planets.push_back(Planet("Saturn", 2.0f, 40.0f, 0.97f, 1.0f, 0.8f, 0.5f));
    planets.push_back(Planet("Uranus", 1.5f, 50.0f, 0.68f, 0.2f, 0.8f, 1.0f));
    planets.push_back(Planet("Neptune", 1.4f, 60.0f, 0.54f, 0.2f, 0.2f, 1.0f));
}

void drawCircle(float radius, int segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, 0, 0);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * PI * i / segments;
        glVertex3f(cos(angle) * radius, sin(angle) * radius, 0);
    }
    glEnd();
}

void drawOrbit(float radius) {
    glDisable(GL_LIGHTING);
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < CIRCLE_SEGMENTS; i++) {
        float angle = 2.0f * PI * i / CIRCLE_SEGMENTS;
        glVertex3f(cos(angle) * radius, 0, sin(angle) * radius);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void drawPlanet(const Planet& planet, float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    
    // Draw planet
    glColor3f(planet.r, planet.g, planet.b);
    glutSolidSphere(planet.radius, 20, 20);
    
    // Draw moons
    for (size_t i = 0; i < planet.moons.size(); i++) {
        const Planet& moon = planet.moons[i];
        float moonX = cos(moon.angle) * moon.distance;
        float moonZ = sin(moon.angle) * moon.distance;
        
        glPushMatrix();
        glTranslatef(moonX, 0, moonZ);
        glColor3f(moon.r, moon.g, moon.b);
        glutSolidSphere(moon.radius, 10, 10);
        glPopMatrix();
        
        // Draw moon orbit
        if (showOrbits) {
            drawOrbit(moon.distance);
        }
    }
    
    glPopMatrix();
}

void drawStars() {
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);
    glPointSize(1.0f);
    
    glBegin(GL_POINTS);
    for (int i = 0; i < 500; i++) {
        float x = (rand() % 2000 - 1000) / 10.0f;
        float y = (rand() % 2000 - 1000) / 10.0f;
        float z = (rand() % 2000 - 1000) / 10.0f;
        if (sqrt(x*x + y*y + z*z) > 70.0f) {
            glVertex3f(x, y, z);
        }
    }
    glEnd();
    
    glEnable(GL_LIGHTING);
}

void drawText(float x, float y, const std::string& text) {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(x, y);
    
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Set camera position
    glTranslatef(0, 0, -cameraDistance);
    glRotatef(cameraAngleX, 1, 0, 0);
    glRotatef(cameraAngleY, 0, 1, 0);
    
    // Draw stars
    drawStars();
    
    // Draw sun
    glPushMatrix();
    glColor3f(sun.r, sun.g, sun.b);
    glutSolidSphere(sun.radius, 30, 30);
    glPopMatrix();
    
    // Draw planets
    for (size_t i = 0; i < planets.size(); i++) {
        Planet& planet = planets[i];
        
        float planetX = cos(planet.angle) * planet.distance;
        float planetZ = sin(planet.angle) * planet.distance;
        
        // Highlight selected planet
        if (selectedPlanet == (int)i) {
            glDisable(GL_LIGHTING);
            glColor3f(1.0f, 1.0f, 0.0f);
            glPushMatrix();
            glTranslatef(planetX, 0, planetZ);
            glutWireSphere(planet.radius * 1.2f, 15, 15);
            glPopMatrix();
            glEnable(GL_LIGHTING);
        }
        
        // Draw planet orbit
        if (showOrbits) {
            drawOrbit(planet.distance);
        }
        
        // Draw planet
        drawPlanet(planet, planetX, 0, planetZ);
    }
    
    // Draw UI
    std::string info = "Controls: WASD/Arrow Keys - Rotate | Mouse - Rotate | Scroll - Zoom";
    drawText(10, WINDOW_HEIGHT - 20, info);
    
    std::string controls = "Space - Pause | O - Toggle Orbits | +/- - Speed | 1-8 - Select Planet";
    drawText(10, WINDOW_HEIGHT - 40, controls);
    
    std::string status = "Speed: " + std::to_string(timeSpeed) + "x";
    if (isPaused) status += " (PAUSED)";
    drawText(10, WINDOW_HEIGHT - 60, status);
    
    if (selectedPlanet >= 0 && selectedPlanet < (int)planets.size()) {
        std::string planetInfo = "Selected: " + planets[selectedPlanet].name;
        drawText(10, WINDOW_HEIGHT - 80, planetInfo);
    }
    
    glutSwapBuffers();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: // Escape
            exit(0);
            break;
        case ' ':
            isPaused = !isPaused;
            break;
        case 'o':
        case 'O':
            showOrbits = !showOrbits;
            break;
        case '+':
        case '=':
            timeSpeed *= 1.2f;
            break;
        case '-':
            timeSpeed /= 1.2f;
            break;
        case 'w':
        case 'W':
            cameraAngleX -= 5;
            break;
        case 's':
        case 'S':
            cameraAngleX += 5;
            break;
        case 'a':
        case 'A':
            cameraAngleY -= 5;
            break;
        case 'd':
        case 'D':
            cameraAngleY += 5;
            break;
        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8':
            selectedPlanet = key - '1';
            if (selectedPlanet >= (int)planets.size()) selectedPlanet = -1;
            break;
        case '0':
            selectedPlanet = -1;
            break;
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            cameraAngleX -= 5;
            break;
        case GLUT_KEY_DOWN:
            cameraAngleX += 5;
            break;
        case GLUT_KEY_LEFT:
            cameraAngleY -= 5;
            break;
        case GLUT_KEY_RIGHT:
            cameraAngleY += 5;
            break;
    }
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mousePressed = true;
            lastMouseX = x;
            lastMouseY = y;
        } else {
            mousePressed = false;
        }
    } else if (button == 3) { // Mouse wheel up
        cameraDistance -= 2.0f;
        if (cameraDistance < 10.0f) cameraDistance = 10.0f;
        glutPostRedisplay();
    } else if (button == 4) { // Mouse wheel down
        cameraDistance += 2.0f;
        if (cameraDistance > 200.0f) cameraDistance = 200.0f;
        glutPostRedisplay();
    }
}

void mouseMotion(int x, int y) {
    if (mousePressed) {
        cameraAngleY += (x - lastMouseX) * 0.5f;
        cameraAngleX += (y - lastMouseY) * 0.5f;
        lastMouseX = x;
        lastMouseY = y;
        glutPostRedisplay();
    }
}

void update(int value) {
    if (!isPaused) {
        globalTime += 0.016f * timeSpeed; // ~60 FPS
        
        // Update planet positions
        for (Planet& planet : planets) {
            planet.angle += planet.speed * 0.01f * timeSpeed;
            if (planet.angle > 2 * PI) planet.angle -= 2 * PI;
            
            // Update moons
            for (Planet& moon : planet.moons) {
                moon.angle += moon.speed * 0.01f * timeSpeed;
                if (moon.angle > 2 * PI) moon.angle -= 2 * PI;
            }
        }
    }
    
    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // ~60 FPS
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Interactive Solar System Simulation");
    
    initOpenGL();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    glutTimerFunc(0, update, 0);
    
    std::cout << "Solar System Simulation Controls:" << std::endl;
    std::cout << "WASD / Arrow Keys - Rotate view" << std::endl;
    std::cout << "Mouse drag - Rotate view" << std::endl;
    std::cout << "Mouse wheel - Zoom in/out" << std::endl;
    std::cout << "Space - Pause/Resume" << std::endl;
    std::cout << "O - Toggle orbit display" << std::endl;
    std::cout << "+/- - Increase/Decrease speed" << std::endl;
    std::cout << "1-8 - Select planet" << std::endl;
    std::cout << "0 - Deselect planet" << std::endl;
    std::cout << "ESC - Exit" << std::endl;
    
    glutMainLoop();
    return 0;
}