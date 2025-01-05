#include "GL/freeglut_std.h"
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PI 3.14159265
#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define G -9.8f   // Constante gravitationelle
#define DT 0.008f // DeltaTime intervalle de temps (~120 FPS)

typedef struct {
  float px, py; // Position
  float vx, vy; // Velocity
  float r;      // Radius
  int n;        // Resolution
} *Circle;

Circle c;
float fa = 10;  // faster animations
float cr = 0.8; // constante rebond

Circle initCircle(float px, float py, float vx, float vy, float r, int n) {
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

  return c;
}

int frameCount = 0;
double previousTime = 0.0;
double fps = 0.0;

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
  glColor3f(0, 0.6, 0);

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

void init() {
  glClearColor(0.0, 0.0, 0.0, 0);
  gluOrtho2D(0, WIN_WIDTH, 0, WIN_HEIGHT);

  c = initCircle(400.0, 300.0, -400.0, 0.0, 20.0, 15);
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
  char pos[32];
  sprintf(pos, "X : %.2f Y : %.2f (pos)", c->px, c->py);
  char vel[64];
  sprintf(vel, "VelX : %.2f VelY : %.2f (px/s)", c->vx, c->vy);

  textRender(10, WIN_HEIGHT - 20, fpsText);
  textRender(10, WIN_HEIGHT - 35, pos);
  textRender(10, WIN_HEIGHT - 50, vel);
}

void updatePos() {
  c->vy += (G * fa) * DT;
  c->vx += 1.0 * DT; 
  c->py += c->vy * DT;
  c->px += c->vx * DT;

  if (c->py <= 0 + c->r || c->py >= WIN_HEIGHT - c->r) {
    c->py = 0 + c->r;
    c->vy = -(c->vy) * cr;
  }

  if (c->px <= 0 + c->r || c->px >= WIN_WIDTH - c->r) {
    printf("rebond sur x : %f\n", c->vx);
    c->vx = -(c->vx) * cr;
  }
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  calculateFPS();

  drawCircle(c);

  showData();

  /*
  glColor3ub(255, 255, 255);
  char fpsText[64];
  sprintf(fpsText, "FPS : %.2f", fps);
  renderText(10, WIN_HEIGHT - 20, fpsText);
  */

  glutSwapBuffers();

  glutPostRedisplay();
}

void timer(int x) {
  updatePos();
  glutPostRedisplay();
  glutTimerFunc(8, timer, 0);
}

int main(int argc, char *argv[]) {

  printf("Lancement programme\n");

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
  glutInitWindowPosition(700, 20);
  glutCreateWindow("Boing");
  init();

  glutDisplayFunc(display);
  glutTimerFunc(0, timer, 0); // Lance le timer

  glutMainLoop();

  printf("Test est ce que on arrive ici ?");

  return 0;
}
