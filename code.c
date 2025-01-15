/* Prochain truc a faire :
 *    - correction colisions entre cercles
 *    - changement pour faire apparaitre cercles au click
 *    - stockage des cercles en linked list
 */

/* =============================== */
/*         INCLUDE & DEFINE        */
/* =============================== */

#include "GL/freeglut_std.h"
#include <GL/glut.h>
#include <OpenGL/OpenGL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PI 3.14159265
#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define G -9.8f
#define DT 0.008f
#define NBR_CIRCLES 2
#define NBR_COLORS 2

/* =============================== */
/*            STRUCTURES           */
/* =============================== */

typedef struct {
  float x, y;
} Vector2D;

typedef struct {
  Vector2D pos;
  Vector2D vel;
  float rad;
  int n;
  double colorShift;
  unsigned char r, g, b;
} *Circle;

/* =============================== */
/*             VARIABLES           */
/* =============================== */

// Tableau de cercles
Circle c[NBR_CIRCLES];

// Variables pour physique des particules
float fa = 20;  // faster animations
float cr = 0.7; // constante rebond

// Variables pour fps
int frameCount = 0;
double previousTime = 0.0;
double fps = 0.0;

/* =============================== */
/*           PROTOTYPES            */
/* =============================== */

// Initialisation
Circle initCircle(float px, float py, float vx, float vy, float rad, int n,
                  int i);
void initTabCircle();
void init();

// Physique et mise à jour
void updatePos(Circle c);
void bounceEdges(Circle c);
void collisionDetection();
void collisionResolution(Circle c1, Circle c2);

// Affichage
void drawCircle(Circle c);
void display();
void calculateFPS();
void textRender(int x, int y, char *text);
void showData();

// Utilitaires
float generateRandomFloat(float min, float max);
int randBetween(int min, int max);
void wavelengthToRgb(double wavelength, unsigned char *r, unsigned char *g,
                     unsigned char *b);
void timer(int x); 

// Vecteurs
float vecDot(Vector2D a, Vector2D b);
Vector2D vecSub(Vector2D a, Vector2D b);
Vector2D vecMul(Vector2D v, float scalar);
Vector2D vecAdd(Vector2D a, Vector2D b);
float vecSq(Vector2D v);

/* =============================== */
/*         FONCTIONS UTILES        */
/* =============================== */

float generateRandomFloat(float min, float max) {
  return min + (float)rand() / RAND_MAX * (max - min);
}

int randBetween(int min, int max) { return min + rand() % (max - min + 1); }

void wavelengthToRgb(double wavelength, unsigned char *r, unsigned char *g,
                     unsigned char *b) {
  double gamma = 0.8, intensity_max = 255, factor = 0.0;
  double red = 0.0, green = 0.0, blue = 0.0;

  if (wavelength >= 380 && wavelength < 440) {
    red = -(wavelength - 440) / (440 - 380);
    blue = 1.0;
  } else if (wavelength >= 440 && wavelength < 490) {
    green = (wavelength - 440) / (490 - 440);
    blue = 1.0;
  } else if (wavelength >= 490 && wavelength < 510) {
    green = 1.0;
    blue = -(wavelength - 510) / (510 - 490);
  } else if (wavelength >= 510 && wavelength < 580) {
    red = (wavelength - 510) / (580 - 510);
    green = 1.0;
  } else if (wavelength >= 580 && wavelength < 645) {
    red = 1.0;
    green = -(wavelength - 645) / (645 - 580);
  } else if (wavelength >= 645 && wavelength <= 750) {
    red = 1.0;
  }

  if (wavelength >= 380 && wavelength < 420) {
    factor = 0.3 + 0.7 * (wavelength - 380) / (420 - 380);
  } else if (wavelength >= 645 && wavelength <= 750) {
    factor = 0.3 + 0.7 * (750 - wavelength) / (750 - 645);
  } else {
    factor = 1.0;
  }

  *r = (unsigned char)(pow(red * factor, gamma) * intensity_max);
  *g = (unsigned char)(pow(green * factor, gamma) * intensity_max);
  *b = (unsigned char)(pow(blue * factor, gamma) * intensity_max);
}

/* =============================== */
/*           VECTEURS              */
/* =============================== */

float vecDot(Vector2D a, Vector2D b) {
    return a.x * b.x + a.y * b.y;
}

Vector2D vecSub(Vector2D a, Vector2D b) {
    Vector2D result = {a.x - b.x, a.y - b.y};
    return result;
}

Vector2D vecMul(Vector2D v, float scalar) {
    Vector2D result = {v.x * scalar, v.y * scalar};
    return result;
}

Vector2D vecAdd(Vector2D a, Vector2D b) {
    Vector2D result = {a.x + b.x, a.y + b.y};
    return result;
}

float vecSq(Vector2D v) {
  return v.x * v.x + v.y * v.y;
}

/* =============================== */
/*         INITIALISATION          */
/* =============================== */

Circle initCircle(float px, float py, float vx, float vy, float rad, int n,
                  int i) {
  Circle c = malloc(sizeof(*c));
  if (!c) {
    fprintf(stderr, "Erreur : allocation echouee\n");
    exit(EXIT_FAILURE);
  }
  c->pos = (Vector2D){px, py};
  c->vel = (Vector2D){vx, vy};
  c->rad = rad;
  c->n = n;
  wavelengthToRgb(380 + (370.0 / NBR_COLORS) * (i % NBR_COLORS), &c->r, &c->g,
                  &c->b);
  return c;
}

void initTabCircle() {
  for (int i = 0; i < NBR_CIRCLES; i++) {
    c[i] = initCircle(
        generateRandomFloat(0, WIN_WIDTH), generateRandomFloat(0, WIN_HEIGHT),
        generateRandomFloat(-1000, 1000), generateRandomFloat(-1000, 1000),
        randBetween(20, 70), 25, i);
  }
}

void init() {
  glClearColor(0.0, 0.0, 0.0, 0);
  gluOrtho2D(0, WIN_WIDTH, 0, WIN_HEIGHT);
  initTabCircle();
}

/* =============================== */
/*        PHYSIQUE & MOUVEMENT     */
/* =============================== */

void bounceEdges(Circle c) {
  // Bounces with walls (X axis)
  if (c->pos.x < c->rad) {
    c->pos.x = c->rad;
    c->vel.x *= -1 * cr;
    c->vel.y *= cr;
  } else if (c->pos.x > WIN_WIDTH - c->rad) {
    c->pos.x = WIN_WIDTH - c->rad;
    c->vel.x *= -1 * cr;
    c->vel.y *= cr;
  }

  // Bounces with floor and ceiling
  if (c->pos.y < c->rad) {
    c->pos.y = c->rad;
    c->vel.y *= -1 * cr;
    c->vel.x *= cr;
  } else if (c->pos.y > WIN_HEIGHT - c->rad) {
    c->pos.y = WIN_HEIGHT - c->rad;
    c->vel.y *= -1 * cr;
    c->vel.x *= cr;
  }

  return;
}


void updatePos(Circle c) {
  // update velocity
  c->vel.y += (G * fa) * DT;
  c->vel.x += 1.0 * DT;

  // update positions
  c->pos.y += c->vel.y * DT;
  c->pos.x += c->vel.x * DT;

  return;
}

void collisionDetection() {
    for (int i = 0; i < NBR_CIRCLES; i++) {
        for (int j = i + 1; j < NBR_CIRCLES; j++) {
            Vector2D delta = vecSub(c[j]->pos, c[i]->pos);
            if (vecSq(delta) <= pow(c[i]->rad + c[j]->rad, 2)) {
                collisionResolution(c[i], c[j]);
            }
        }
    }
}

void collisionResolution(Circle c1, Circle c2) {

  float mass =
      PI *
      ((c1->rad * c1->rad) +
       (c2->rad *
        c2->rad)); // somme de la masse des deux balles (base sur le radius)

  printf("boing \n");

  return;
}

/* =============================== */
/*           AFFICHAGE             */
/* =============================== */

void drawCircle(Circle c) {
    glColor3ub(c->r, c->g, c->b);
    float a = (2 * PI) / c->n;
    float px2 = c->pos.x + c->rad;
    float py2 = c->pos.y;

    glBegin(GL_TRIANGLES);
    for (int i = 1; i <= c->n; i++) {
        glVertex2f(c->pos.x, c->pos.y);
        glVertex2f(px2, py2);
        px2 = c->pos.x + c->rad * cos(a * i);
        py2 = c->pos.y + c->rad * sin(a * i);
        glVertex2f(px2, py2);
    }
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    calculateFPS();
    for (int i = 0; i < NBR_CIRCLES; i++) {
        drawCircle(c[i]);
    }
    showData();
    glutSwapBuffers();
}

void calculateFPS() {
    static int frameCount = 0;
    static double previousTime = 0.0;
    double currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    frameCount++;
    if (currentTime - previousTime >= 1.0) {
        fps = frameCount / (currentTime - previousTime);
        previousTime = currentTime;
        frameCount = 0;
    }
}

void textRender(int x, int y, char *text) {
    glRasterPos2i(x, y);
    for (const char *c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
}

void showData() {
    glColor3ub(255, 255, 255);
    char fpsText[32];
    sprintf(fpsText, "FPS : %d", (int)fps);
    textRender(10, WIN_HEIGHT - 20, fpsText);
}

/* =============================== */
/*            TIMER                */
/* =============================== */

void timer(int x) {
    for (int i = 0; i < NBR_CIRCLES; i++) {
        updatePos(c[i]);
        bounceEdges(c[i]);
    }
    collisionDetection();
    glutPostRedisplay();
    glutTimerFunc((int)(DT * 1000), timer, 0);
}

/* =============================== */
/*             MAIN                */
/* =============================== */

int main(int argc, char *argv[]) {
    srand((unsigned int)time(NULL));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
    glutCreateWindow("Boing");
    init();
    glutTimerFunc(0, timer, 0);
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}
