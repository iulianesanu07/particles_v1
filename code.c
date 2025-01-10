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
#define G -9.8f               // Constante gravitationelle
#define DT 0.008f             // DeltaTime intervalle de temps (~120 FPS)
#define NBR_CIRCLES 200        // Nombre de cercles
#define PI_RGB 2.09439510239  // 2PI/3 Pour avoir une division d'un cercle en 3 (donc rgb)
#define NBR_COLORS 100         // Nombre de couleurs voulues

/* Declaration structure d'un Cercle */
typedef struct {
  float px, py;       // Position
  float vx, vy;       // Velocity
  float r;            // Radius
  int n;              // Resolution
  float colorShift;   // Bon bah c'est explicite la x)
} *Circle;

/* Variables pour cercles */
Circle c[NBR_CIRCLES];
unsigned long long int shifter = 0;   // Parce que j'ai un trop petit cerveau pour faire autrement
                                      // Et parce que la demesure c'est marrant
                                      // 0 -> 18.446.744.073.709.551.615

/* Variables pour physique des particules */
float fa = 20;  // faster animations
float cr = 0.7; // constante rebond

/* Variables pour fps */
int frameCount = 0;
double previousTime = 0.0;
double fps = 0.0;

/* Prototypes */
Circle initCircle(float px, float py, float vx, float vy, float r, int n, int i);
void calculateFPS();
void drawCircle(Circle c);
void initTabCircle(int n);
void init();
void textRender(int x, int y, char *text);
void showData();
void updatePos(Circle c);
void display();
void timer(int x);
float generateRandomFloat(float min, float max);
int randBetween(int min, int max);
void collisionEntreBalles();
void numBalle();

/* Main */

float generateRandomFloat(float min, float max) {
  return min + (float)rand() / RAND_MAX * (max - min);
}

int randBetween(int min, int max) { return min + rand() % (max - min + 1); }

Circle initCircle(float px, float py, float vx, float vy, float r, int n, int i) {
  Circle c = malloc(sizeof(*c));

  if (!c) {
    fprintf(stderr, "Erreur : allocation de memoire pour 'Circle' echouee\n");
    exit(EXIT_FAILURE);
  }

  c->px = px;
  c->py = py;
  c->vx = vx;
  c->vy = vy;
  c->r = r;
  c->n = n;
  c->colorShift = i % NBR_COLORS;

  printf("%d\n", i);

  return c;
}

void calculateFPS() {
  double currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0; // temps en secondes
  frameCount++;

  if (currentTime - previousTime >= 1.0) {
    fps = frameCount / (currentTime - previousTime);
    previousTime = currentTime;
    frameCount = 0;
  }
}

/* Inutilise mais on sait jamais, ca peut toujours servir
void renderText(int x, int y, const char *text) {
  glRasterPos2i(x, y);
  for(const char *c = text; *c !='\0'; c++) {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
  }
}
*/

void drawCircle(Circle c) {

  float r = sin(c->colorShift) * 255;
  float g = sin(c->colorShift + PI_RGB) * 255;
  float b = sin(c->colorShift - PI_RGB) * 255;

  glColor3ub(r, g, b);

  float a = (2 * PI) / c->n;

  float px2 = c->px + c->r;
  float py2 = c->py;

  glBegin(GL_TRIANGLES);
  for (int i = 1; i <= c->n; i++) {
    glVertex2f((float)c->px, (float)c->py);
    glVertex2f(px2, py2);
    px2 = c->px + c->r * cos(a * i);
    py2 = c->py + c->r * sin(a * i);
    glVertex2f(px2, py2);
  }
  glEnd();
}

void initTabCircle(int n) {
  for (int i = 0; i < NBR_CIRCLES; i++) {
    c[i] = initCircle(generateRandomFloat(0, WIN_WIDTH),
                      generateRandomFloat(0, WIN_HEIGHT),
                      generateRandomFloat(-100, 100),
                      generateRandomFloat(-100, 100), 
                      randBetween(5, 15), 
                      25,
                      i);
  }
}

void init() {
  glClearColor(0.0, 0.0, 0.0, 0);
  gluOrtho2D(0, WIN_WIDTH, 0, WIN_HEIGHT);

  initTabCircle(1);

  printf("Fin initialisation cercles\n");
  // c[n] = initCircle(400.0, 300.0, vx, vy, 20.0, 15);      // parametres
  // initiaux
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
  /*
    char pos[32];
    sprintf(pos, "X : %.2f Y : %.2f (pos)", c->px, c->py);
    char vel[64];
    sprintf(vel, "VelX : %.2f VelY : %.2f (px/s)", c->vx, c->vy);
  */

  textRender(10, WIN_HEIGHT - 20, fpsText);
  /*
    textRender(10, WIN_HEIGHT - 35, pos);
    textRender(10, WIN_HEIGHT - 50, vel);
  */
}

void updatePos(Circle c) {
  // update velocity
  c->vy += (G * fa) * DT;
  c->vx += 1.0 * DT;

  // update positions
  c->py += c->vy * DT;
  c->px += c->vx * DT;

  // collisions on Y
  if (c->py - c->r < 0) {
    c->py = c->r;
    c->vy = -(c->vy) * cr;
    c->vx *= cr;
  } else if (c->py + c->r > WIN_HEIGHT) {
    c->py = WIN_HEIGHT - c->r;
    c->vy = -(c->vy) * cr;
    c->vx *= cr;
  }

  // collisions on X
  if (c->px - c->r < 0) {
    c->px = c->r;
    c->vx = -(c->vx) * cr;
    c->vy *= cr;
  } else if (c->px + c->r > WIN_WIDTH) {
    c->px = WIN_WIDTH - c->r;
    c->vx = -(c->vx) * cr;
    c->vy *= cr;
  }
}

void collisionEntreBalles() {
  for (int i = 0; i < NBR_CIRCLES; i++) {
    for (int j = i + 1; j < NBR_CIRCLES; j++) {
      if (c[i] != c[j]) {
        float dist = sqrt(((c[j]->px - c[i]->px) * (c[j]->px - c[i]->px)) +
                          ((c[j]->py - c[i]->py) * (c[j]->py - c[i]->py)));
        if ((c[i]->r + c[j]->r) >= dist) {
          c[i]->vx = -c[i]->vx;
          c[i]->vy = -c[i]->vy;
          c[j]->vx = -c[j]->vx;
          c[j]->vy = -c[j]->vy;
        }
      }
    }
  }
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  calculateFPS();

  for (int i = 0; i < NBR_CIRCLES; i++) {
    drawCircle(c[i]);
  }

  showData();

  /*
  glColor3ub(255, 255, 255);
  char fpsText[64];
  sprintf(fpsText, "FPS : %.2f", fps);
  renderText(10, WIN_HEIGHT - 20, fpsText);
  */

  glutSwapBuffers();
}

void timer(int x) {

  for (int i = 0; i < NBR_CIRCLES; i++) {
    updatePos(c[i]);
  }

  collisionEntreBalles();

  glutPostRedisplay();
  glutTimerFunc(8, timer, 0);
}

/*
void mouseClick(int button, int state, int x, int y) {
  if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
    c->px = x;
    c->py = WIN_HEIGHT -y;    // because of the inversion of the y axis
    c->vx = 0;
    c->vy = 0;
    glutPostRedisplay();
  }
}
*/

int main(int argc, char *argv[]) {

  printf("Lancement programme :\n");

  srand((unsigned int)time(NULL));

  /*
  float vxI = strtof(argv[1], NULL);
  float vyI = strtof(argv[2], NULL);

  printf("argv[0] : %s\n", argv[0]);
  printf("argv[1] : %f\n", vxI);
  printf("argv[2] : %f\n", vyI);
  */

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
  glutInitWindowPosition(500, -500);
  glutCreateWindow("Boing");
  init();

  glutTimerFunc(0, timer, 0); // Lance le timer
                              // glutMouseFunc(mouseClick);

  glutDisplayFunc(display);

  glutMainLoop();

  printf("Test est ce que on arrive ici ?");

  return 0;
}
