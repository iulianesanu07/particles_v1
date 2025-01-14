/* Prochain truc a faire : 
 *    - correction colisions entre cercles
 *    - changement pour faire apparaitre cercles au click
 *    - stockage des cercles en linked list
 */




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
#define G -9.8f       // Constante gravitationelle
#define DT 0.008f     // DeltaTime intervalle de temps (~120 FPS)
#define NBR_CIRCLES 5 // Nombre de cercles
#define PI_RGB                                                                 \
  2.09439510239 // 2PI/3 Pour avoir une division d'un cercle en 3 (donc rgb)
#define NBR_COLORS 5 // Nombre de couleurs voulues

/* Declaration structure d'un Cercle */
typedef struct {
  float px, py;          // Position
  float vx, vy;          // Velocity
  float rad;             // Radius
  int n;                 // Resolution
  double colorShift;     // Bon bah c'est explicite la x)
  unsigned char r, g, b; // Colors value
} *Circle;

/* Variables pour cercles */
Circle c[NBR_CIRCLES];
unsigned long long int shifter =
    0; // Parce que j'ai un trop petit cerveau pour faire autrement
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
Circle initCircle(float px, float py, float vx, float vy, float r, int n,
                  int i);
void calculateFPS();
void drawCircle(Circle c);
void initTabCircle();
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
void bounceEdges(Circle c);

/* Main */

float generateRandomFloat(float min, float max) {
  return min + (float)rand() / RAND_MAX * (max - min);
}

int randBetween(int min, int max) { return min + rand() % (max - min + 1); }

void wavelengthToRgb(double wavelength, unsigned char *r, unsigned char *g,
                     unsigned char *b) {

  double gamma = 0.8;
  double intensity_max = 255;
  double factor = 0.0;
  double red = 0.0, green = 0.0, blue = 0.0;

  if (wavelength >= 380 && wavelength < 440) {
    red = -(wavelength - 440) / (440 - 380);
    green = 0.0;
    blue = 1.0;
  } else if (wavelength >= 440 && wavelength < 490) {
    red = 0.0;
    green = (wavelength - 440) / (490 - 440);
    blue = 1.0;
  } else if (wavelength >= 490 && wavelength < 510) {
    red = 0.0;
    green = 1.0;
    blue = -(wavelength - 510) / (510 - 490);
  } else if (wavelength >= 510 && wavelength < 580) {
    red = (wavelength - 510) / (580 - 510);
    green = 1.0;
    blue = 0.0;
  } else if (wavelength >= 580 && wavelength < 645) {
    red = 1.0;
    green = -(wavelength - 645) / (645 - 580);
    blue = 0.0;
  } else if (wavelength >= 645 && wavelength <= 750) {
    red = 1.0;
    green = 0.0;
    blue = 0.0;
  }

  // Ajustement de l'intensité
  if (wavelength >= 380 && wavelength < 420) {
    factor = 0.3 + 0.7 * (wavelength - 380) / (420 - 380);
  } else if (wavelength >= 645 && wavelength <= 750) {
    factor = 0.3 + 0.7 * (750 - wavelength) / (750 - 645);
  } else {
    factor = 1.0;
  }

  // Appliquer le gamma et normaliser
  *r = (unsigned char)(pow(red * factor, gamma) * intensity_max);
  *g = (unsigned char)(pow(green * factor, gamma) * intensity_max);
  *b = (unsigned char)(pow(blue * factor, gamma) * intensity_max);
}

/* Fonction d'initialisation d'un cercle
 * Parametres : 
 *  float px, py  -> position centre
 *  float vx, vy  -> vecteur vitesse
 *  float rad     -> rayon
 *  int n         -> resolution
 *  int i         -> numerotation cercle
 *  */
Circle initCircle(float px, float py, float vx, float vy, float rad, int n,
                  int i) {
  Circle c = malloc(sizeof(*c));

  if (!c) {
    fprintf(stderr, "Erreur : allocation de memoire pour 'Circle' echouee\n");
    exit(EXIT_FAILURE);
  }

  c->px = px;
  c->py = py;
  c->vx = vx;
  c->vy = vy;
  c->rad = rad;
  c->n = n;

  double colorShift =
      380 +
      (370.0 / NBR_COLORS) *
          (i % NBR_COLORS); // 380 -> minwavelength => plage : 380-750 donc 370

  wavelengthToRgb(colorShift, &c->r, &c->g, &c->b);

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

//unsigned char r, g, b;

//wavelengthToRgb(c->colorShift, &r, &g, &b);

  glColor3ub(c->r, c->g, c->b);

  float a = (2 * PI) / c->n;

  float px2 = c->px + c->rad;
  float py2 = c->py;

  glBegin(GL_TRIANGLES);
  for (int i = 1; i <= c->n; i++) {
    glVertex2f((float)c->px, (float)c->py);
    glVertex2f(px2, py2);
    px2 = c->px + c->rad * cos(a * i);
    py2 = c->py + c->rad * sin(a * i);
    glVertex2f(px2, py2);
  }
  glEnd();
}

void initTabCircle() {
  for (int i = 0; i < NBR_CIRCLES; i++) {
    c[i] = initCircle(
        generateRandomFloat(0, WIN_WIDTH), generateRandomFloat(0, WIN_HEIGHT),
        generateRandomFloat(-100, 100), generateRandomFloat(-100, 100),
        randBetween(20, 70), 25, i);
  }
}



void init() {
  glClearColor(0.0, 0.0, 0.0, 0);
  gluOrtho2D(0, WIN_WIDTH, 0, WIN_HEIGHT);

  initTabCircle();

  printf("Fin initialisation cercles\n");
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

  return;
}

void bounceEdges(Circle c) {
  // Bounces with walls (X axis)
  if (c->px < c->rad) {
    c->px = c->rad;
    c->vx *= -1 * cr;
    c->vy *= cr;
  } else if (c->px > WIN_WIDTH - c->rad) {
    c->px = WIN_WIDTH - c->rad;
    c->vx *= -1 * cr;
    c->vy *= cr;
  }

  // Bounces with floor and ceiling
  if (c->py < c->rad) {
    c->py = c->rad;
    c->vy *= -1 * cr;
    c->vx *= cr;
  } else if (c->py > WIN_HEIGHT - c->rad) {
    c->py = WIN_HEIGHT - c->rad;
    c->vy *= -1 * cr;
    c->vx *= cr;
  }

  return;
}

void collisionEntreBalles() {
  for (int i = 0; i < NBR_CIRCLES; i++) {
    for (int j = i + 1; j < NBR_CIRCLES; j++) {
      if (c[i] != c[j]) {
        float dist = sqrt(((c[j]->px - c[i]->px) * (c[j]->px - c[i]->px)) +
                          ((c[j]->py - c[i]->py) * (c[j]->py - c[i]->py)));
        if ((c[i]->rad + c[j]->rad) >= dist) {
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
    bounceEdges(c[i]);
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
