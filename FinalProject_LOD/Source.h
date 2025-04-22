#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <iostream>
#include <string>
// #include <fstream>

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


// GLOBALS
static int win;
static float viewDist;


// HELPERS/HANDLERS
void loadMeshFile(const char filePath[]);
void normalizeVertices();


// OPENGL FUNCS
void OpenGLInit();
void display();


