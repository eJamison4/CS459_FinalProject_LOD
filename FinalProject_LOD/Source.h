#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <iostream>
#include <string>
#include <fstream>

struct point {
	float x, y, z;
	std::string toString() const {
		return "("
			+ std::to_string(x) + ", "
			+ std::to_string(y) + ", "
			+ std::to_string(z) + ")";
	}
	float max() const { return std::max(x, std::max(y, z)); }
	float min() const { return std::min(x, std::min(y, z)); }
};

struct triFace {
	point *a, *b, *c;
	std::string toString() const {
		return "Face: " + a->toString() + "\n"
			"      " + b->toString() + "\n"
			"      " + c->toString() + "\n";
	}
};

// CONSTANTS
const char meshFilePath[] = "";
const float ANGFACT = { 2.0f };
const int WINDOWPOS[2] = { 50, 50 };
const int WINDOWSIZE[2] = { 1500, 800 };
const float CLEAR[4] = {0.0f, 0.0f, 0.0f, 0.0f};
const float CLEAR_DEPTH = 1.0f;

// GLOBALS
bool mouseDown = false;
int mousex;
int mousey;

static int win;
static float viewDist;
float maxVertex = 0.0f;
float minVertex = 0.0f;
int numVertices = 0;
int numFaces = 0;

point* vertices;
triFace* faces;

float xrot = 0.0f;
float yrot = 0.0f;
float xdiff = 0.0f;
float ydiff = 0.0f;


// HELPERS/HANDLERS
void loadMeshFile_triangular(const char filePath[]);
void normVerticesToRange(const float min, const float max);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void mouseMotion(int x, int y);

// OPENGL FUNCS
void OpenGLInit();
void display();
void resize(int w, int h);
void drawMesh();
void drawLighting();

