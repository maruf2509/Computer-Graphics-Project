#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cstdio>

// Structure to represent a mosquito
struct Mosquito {
    float x, y;       // Position
    float dx, dy;     // Direction of movement
    float size;       // Size of the mosquito
    bool alive;       // Whether the mosquito is alive
    float deathTimer; // Timer for death animation
};

const int NUM_MOSQUITOES = 35; // More mosquitoes
Mosquito mosquitoes[NUM_MOSQUITOES];
int aliveMosquitoes = NUM_MOSQUITOES; // Track alive mosquitoes

// Variables for pond, water bowl, and spray
bool waterBowlVisible = false;
bool spraying = false;
float sprayX = 0.0f, sprayY = 0.0f, sprayRadius = 0.08f;
float waterBowlX = -0.4f, waterBowlY = -0.9f, waterBowlRadius = 0.05f;

// Function to initialize mosquitoes with random positions and directions
void initializeMosquitoes() {
    srand(static_cast<unsigned>(time(0)));
    aliveMosquitoes = NUM_MOSQUITOES;
    for (int i = 0; i < NUM_MOSQUITOES; i++) {
        mosquitoes[i].x = ((rand() % 200) / 100.0f) - 1.0f; // Random x position (-1 to 1)
        mosquitoes[i].y = ((rand() % 200) / 100.0f) - 1.0f; // Random y position (-1 to 1)
        mosquitoes[i].dx = ((rand() % 50) / 10000.0f) - 0.005f; // Slow random x direction
        mosquitoes[i].dy = ((rand() % 50) / 10000.0f) - 0.005f; // Slow random y direction
        mosquitoes[i].size = 0.05f; // Fixed small size
        mosquitoes[i].alive = true; // All mosquitoes start alive
        mosquitoes[i].deathTimer = 0.0f;
    }
}

// Function to check if a mosquito is within spray range
bool isInSprayRange(float mx, float my, float sx, float sy, float radius) {
    float distance = sqrt((mx - sx) * (mx - sx) + (my - sy) * (my - sy));
    return distance <= radius;
}

// Function to draw a small mosquito
void drawMosquito(float x, float y, float size, float alpha = 1.0f) {
    // Body
    glColor4f(0.0f, 0.0f, 0.0f, alpha); // Black with alpha
    glBegin(GL_LINES);
    glVertex2f(x - size / 2, y);
    glVertex2f(x + size / 2, y); // Body line
    glEnd();

    // Head
    glColor4f(0.2f, 0.2f, 0.2f, alpha);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x - size / 2 - size / 4, y);
    for (int i = 0; i <= 360; i++) {
        float angle = i * (3.14159f / 180.0f);
        glVertex2f(x - size / 2 - size / 4 + (size / 4) * cos(angle),
            y + (size / 4) * sin(angle));
    }
    glEnd();

    // Wings
    glColor4f(0.5f, 0.5f, 0.5f, alpha);
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y);
    glVertex2f(x - size * 1.5f, y + size);
    glVertex2f(x - size / 2, y); // Left wing
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(x, y);
    glVertex2f(x + size * 1.5f, y + size);
    glVertex2f(x + size / 2, y); // Right wing
    glEnd();

    // Proboscis
    glColor4f(0.0f, 0.0f, 0.0f, alpha);
    glBegin(GL_LINES);
    glVertex2f(x - size / 2 - size / 4, y);
    glVertex2f(x - size / 2 - size / 2, y); // Proboscis
    glEnd();
}

// Function to draw a house
void drawHouse(float x, float y, float width, float height) {
    // Base of the house
    glColor3f(0.55f, 0.27f, 0.07f); // Dark brown
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();

    // Roof of the house
    glColor3f(0.0f, 0.0f, 0.5f); // Dark blue
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y + height);
    glVertex2f(x + width / 2, y + height + height / 2);
    glVertex2f(x + width, y + height);
    glEnd();
}

// Function to draw a tree
void drawTree(float x, float y) {
    // Tree trunk
    glColor3f(0.54f, 0.27f, 0.07f); // Brown
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + 0.05f, y);
    glVertex2f(x + 0.05f, y + 0.3f);
    glVertex2f(x, y + 0.3f);
    glEnd();

    // Tree leaves
    glColor3f(0.0f, 0.5f, 0.0f); // Green
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 0.1f, y + 0.3f);
    glVertex2f(x + 0.15f, y + 0.5f);
    glVertex2f(x + 0.3f, y + 0.3f); // Top triangle
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(x - 0.1f, y + 0.45f);
    glVertex2f(x + 0.15f, y + 0.7f);
    glVertex2f(x + 0.3f, y + 0.45f); // Bottom triangle
    glEnd();
}

// Function to draw a pond
void drawPond() {
    glColor3f(0.0f, 0.0f, 1.0f); // Blue color for water
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++) {
        float angle = i * 3.14159f / 180.0f;
        glVertex2f(0.7f + 0.3f * cos(angle), -0.85f + 0.2f * sin(angle)); // Pond shape
    }
    glEnd();
}

// Function to get random spray position near houses
void getHouseSprayPosition(float& x, float& y) {
    int houseChoice = rand() % 3; // Choose one of 3 houses
    switch (houseChoice) {
    case 0: // House 1 area
        x = -0.9f + (rand() % 40) / 100.0f; // Around house 1 (-0.9 to -0.5)
        y = -0.8f + (rand() % 40) / 100.0f; // Around house 1 base
        break;
    case 1: // House 2 area  
        x = -0.5f + (rand() % 50) / 100.0f; // Around house 2 (-0.5 to 0.0)
        y = -0.8f + (rand() % 50) / 100.0f; // Around house 2 base
        break;
    case 2: // House 3 area
        x = 0.0f + (rand() % 35) / 100.0f; // Around house 3 (0.0 to 0.35)
        y = -0.8f + (rand() % 35) / 100.0f; // Around house 3 base
        break;
    }
}
void updateMosquitoes() {
    for (int i = 0; i < NUM_MOSQUITOES; i++) {
        if (mosquitoes[i].alive) {
            mosquitoes[i].x += mosquitoes[i].dx;
            mosquitoes[i].y += mosquitoes[i].dy;

            // Reverse direction if mosquito hits a boundary
            if (mosquitoes[i].x < -1.0f || mosquitoes[i].x > 1.0f)
                mosquitoes[i].dx = -mosquitoes[i].dx;
            if (mosquitoes[i].y < -1.0f || mosquitoes[i].y > 1.0f)
                mosquitoes[i].dy = -mosquitoes[i].dy;

            // Check if mosquito is hit by spray
            if (spraying && isInSprayRange(mosquitoes[i].x, mosquitoes[i].y, sprayX, sprayY, sprayRadius)) {
                mosquitoes[i].alive = false;
                mosquitoes[i].deathTimer = 1.0f; // Start death animation
                aliveMosquitoes--;
            }
        }
        else if (mosquitoes[i].deathTimer > 0.0f) {
            // Handle death animation
            mosquitoes[i].deathTimer -= 0.05f;
            if (mosquitoes[i].deathTimer <= 0.0f) {
                mosquitoes[i].deathTimer = 0.0f;
            }
        }
    }
}

// Function to display text on the screen
void displayText(const char* text, float x, float y) {
    glColor3f(0.0f, 0.0f, 0.0f); // Black text
    glRasterPos2f(x, y);
    for (int i = 0; i < strlen(text); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

// Function to display instructions
void displayInstructions() {
    // Define the multiple lines of text
    const char* instructions[] = {
        "Please Press :",
        " S: Start Spray Effect",
        " R: Remove Water from the Bowl",
        " N: Reset Mosquitoes",
        "",
        "Instructions:",
        "1. Keep water clean,",
        "2. cover containers,",
        "3. wear protective clothes."
    };

    float yPos = 0.9f;
    for (int i = 0; i < 9; i++) {
        displayText(instructions[i], 0.3f, yPos);
        yPos -= 0.05f;  // Move to the next line
    }

    // Display mosquito count
    char mosquitoCount[50];
    sprintf(mosquitoCount, "Alive Mosquitoes: %d", aliveMosquitoes);
    displayText(mosquitoCount, 0.3f, 0.4f);
}

// Function to draw clouds in the sky
void drawCloud(float x, float y) {
    glColor3f(1.0f, 1.0f, 1.0f); // White
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i += 10) {
        float angle = i * 3.14159f / 180.0f;
        glVertex2f(x + 0.1f * cos(angle), y + 0.1f * sin(angle));
    }
    glEnd();
}

// Display function
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw background
    glColor3f(0.53f, 0.81f, 0.92f); // Sky blue
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    // Draw clouds
    drawCloud(-0.8f, 0.6f);
    drawCloud(0.2f, 0.8f);
    drawCloud(0.6f, 0.5f);

    // Draw houses
    drawHouse(-0.9f, -0.8f, 0.3f, 0.3f); // House 1
    drawHouse(-0.5f, -0.8f, 0.4f, 0.4f); // House 2
    drawHouse(0.0f, -0.8f, 0.25f, 0.25f); // House 3

    // Draw trees
    drawTree(-0.9f, -0.5f);
    drawTree(-0.6f, -0.6f);
    drawTree(0.8f, -0.7f);

    // Draw pond
    drawPond();

    // Draw mosquitoes
    for (int i = 0; i < NUM_MOSQUITOES; i++) {
        if (mosquitoes[i].alive) {
            drawMosquito(mosquitoes[i].x, mosquitoes[i].y, mosquitoes[i].size);
        }
        else if (mosquitoes[i].deathTimer > 0.0f) {
            // Draw dying mosquito with fading effect
            float alpha = mosquitoes[i].deathTimer;
            drawMosquito(mosquitoes[i].x, mosquitoes[i].y, mosquitoes[i].size, alpha);
        }
    }

    // Draw water bowl if visible
    if (!waterBowlVisible) {
        glColor3f(0.0f, 0.0f, 1.0f);  // Blue water bowl
        glBegin(GL_POLYGON);
        for (int i = 0; i < 360; i++) {
            float angle = i * 3.14159f / 180.0f;
            glVertex2f(waterBowlX + waterBowlRadius * cos(angle), waterBowlY + waterBowlRadius * sin(angle));
        }
        glEnd();
    }

    // Draw spray effect if active
    if (spraying) {
        glColor4f(0.1f, 0.5f, 1.0f, 0.6f);  // Light blue spray with transparency
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(sprayX, sprayY);
        for (int i = 0; i <= 360; i++) {
            float angle = i * 3.14159f / 180.0f;
            glVertex2f(sprayX + sprayRadius * cos(angle), sprayY + sprayRadius * sin(angle));
        }
        glEnd();
        sprayRadius += 0.01f;
        if (sprayRadius > 0.2f) {
            spraying = false;
        }
    }

    // Display text
    displayText("Dengue Awareness: Mosquitoes", -0.9f, 0.9f);

    // Display instructions
    displayInstructions();

    glDisable(GL_BLEND);
    glutSwapBuffers();
}

// Timer function for animation
void timer(int value) {
    updateMosquitoes();      // Update positions
    glutPostRedisplay();     // Redraw the scene
    glutTimerFunc(50, timer, 0); // Approx 20 FPS
}

// Keyboard function
void keyboard(unsigned char key, int x, int y) {
    if (key == 's' || key == 'S') {
        // Start spraying near houses only
        getHouseSprayPosition(sprayX, sprayY);
        sprayRadius = 0.08f;
        spraying = true;
    }

    if (key == 'r' || key == 'R') {
        // Remove water bowl
        waterBowlVisible = true;
    }

    if (key == 'n' || key == 'N') {
        // Reset mosquitoes
        initializeMosquitoes();
    }
}

// Initialization
void init() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // White background
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0); // 2D orthographic projection
    initializeMosquitoes();
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Enhanced Dengue Mosquito Control with Spray Effect");

    init();

    glutDisplayFunc(display);
    glutTimerFunc(50, timer, 0); // Start timer with 50ms interval
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}