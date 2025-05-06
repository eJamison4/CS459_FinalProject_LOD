#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <meshoptimizer.h>
//Make sure to add "\FinalProject_LOD\meshoptimizer\src" into additional include directories under properties:c++

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"



struct point {
	float x, y, z;
};
/*
struct triFace {
	triFace() {a = NULL; b = NULL; c = NULL;}
	triFace(point* p1, point* p2, point* p3) { a = p1; b = p2; c = p3; }
	point* a, * b, * c;
	point* select = a;
	point* next() {
		if (select == a) { select = b; return b; }
		if (select == b) { select = c; return c; }
		if (select == c) { select = a; return a; }
	}
	int operator==(triFace rhs) {
		// returns a 3bit mask where bit1: a==a, bit2: b==b, bit3: c==c
		return 0b000 | (a == rhs.a) | ((b == rhs.b) << 1) | ((c == rhs.c) << 2);
	}
	point mid() const {
		return point(
			(a->x + b->x + c->x) / 3, 
			(a->y + b->y + c->y) / 3,
			(a->z + b->z + c->z) / 3
	)}
	std::string toString() const {
		return "Face: " + a->toString() + "\n"
			"      " + b->toString() + "\n"
			"      " + c->toString() + "\n";
	}
};
*/
// CONSTANTS
const char meshFilePath[] = "";
const float ANGFACT = { 2.0f };
const float SUBSAMPLEFACTOR = 2.0f;
const float ZFACT = { 0.04f };
const int WINDOWPOS_X = 50;
const int WINDOWPOS_Y = 50;
const int WINDOWSIZE_X = 1850;
const int WINDOWSIZE_Y = 950;
const float CLEAR[4] = {0.0f, 0.0f, 0.0f, 0.0f};
const float CLEAR_DEPTH = 1.0f;
const int NUMDRAWSUNTILNEXTSAMPLE = 100;

enum LOD_MODE {
	NONE, DOWNSAMPLE,
};

int lod_mode = NONE;

// GLOBALS
bool mouseDepthMode = false;
bool decoupleCamDepth = false;
float lockedDepth = 0.0f;
bool mouseDown = false;
GLfloat light[4] = { 0.0, 0.0, 0.9, 0.9 };
GLfloat ambLight[4] = { 0.0, 0.0, 0.0, 0.9 };
int mousex;
int mousey;

static int win;
static float viewDist;
float maxVertex = 0.0f;
float minVertex = 0.0f;
size_t numFaces = 0;
int lastSampleDepthRatio = 0;
unsigned int sampleDebounce = 0;

point* originalVertices;
size_t numOrigVertices = 0;
point* vertices;
size_t numVertices = 0;
point* modifiedVertices;
size_t numModdedVertices = 0;

std::vector<unsigned int> remap;

unsigned int* originalIndices;
size_t numIndices = 0;

unsigned int* vertexIndexBuffer;
size_t indexCount = 0;

unsigned int* simplifiedIndexBuffer;
size_t numSimpleIndices = 0;

float xrot = 0.0f;
float yrot = 0.0f;
float tz = 0.0f;
float xdiff = 0.0f;
float ydiff = 0.0f;
float zdiff = 3.0f;
int manual_lod_depth = 0;

// HELPERS/HANDLERS
bool init(const char filePath[]);
void loadMeshFile_triangular(const char filePath[]);
// void normVerticesToRange(const float min, const float max);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void mouseMotion(int x, int y);

// OPENGL FUNCS
void OpenGLInit();
void display();
void resize(int w, int h);
// triFace join(triFace& f1, triFace& f2, triFace& f3);
// void procBatch(std::list<triFace> batch);
void handleMeshSimplify();
void indexMesh();
void drawSimplified();
void drawMesh();
void drawLighting();


// OTHERS
void onExitFree();
void raise(const char msg[]) {
	printf("\nRAISED EXCEPTION: '%s'\n", msg);
	exit(-1);
}
