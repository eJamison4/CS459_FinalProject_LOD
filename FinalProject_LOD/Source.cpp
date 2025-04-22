#include "Source.h"


int main(int argc, char* argv[]) {
	glutInit(&argc, argv);

	glutInitWindowPosition(WINDOWPOS[0], WINDOWPOS[1]);
	glutInitWindowSize(WINDOWSIZE[0], WINDOWSIZE[1]);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	win = glutCreateWindow("LOD Algorithms Demo");
	OpenGLInit();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(resize);

	glutMainLoop();
	free(faces);
	free(vertices);

	return 0;
}

void disply() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	// gluLookAt();

	glRotatef(xrot, 1.0f, 0.0f, 0.0f);
	glRotatef(yrot, 0.0f, 1.0f, 0.0f);

	drawMesh();
	drawLighting();

	glFlush();
	glutSwapBuffers();
}

void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		mouseDown = true;
		xdiff = x - yrot;
		ydiff = -y + xrot;
	}
	else mouseDown = false;
}

void mouseMotion(int x, int y) {
	if (mouseDown) {
		xrot += (ANGFACT * (y - mousey));
		yrot += (ANGFACT * (x - mousex));
		mousex = x;
		mousey = y;
		glutPostRedisplay();
	}
}

void OpenGLInit() {
	glShadeModel(GL_SMOOTH);
	glClearColor(CLEAR[0], CLEAR[1], CLEAR[2], CLEAR[3]);
	glClearDepth(CLEAR_DEPTH);
	glEnable(GL_DEPTH_TEST);
}

void loadMeshFile_triangular(const char filePath[]) {
	std::ifstream infile(filePath);
	std::string line;
	std::getline(infile, line);
	if ((line > "OFF") != 0) throw("Opened file is not an .OFF file...\n");

	int _zero; // not used
	infile >> numVertices >> numFaces >> _zero;

	vertices = (point*)calloc(numVertices, 3 * sizeof(float));
	if (!vertices) throw("Failed mem alloc for vertices...\n");

	faces = (triFace*)calloc(numFaces, 3 * sizeof(point*));
	if (!faces) throw("Not enough memory available to allocate faces array...\n");

	for (int i = 0; i < numVertices; i++) {
		infile >> vertices[i].x >> vertices[i].y >> vertices[i].z;
		maxVertex = std::max(maxVertex, vertices[i].max());
		minVertex = std::min(minVertex, vertices[i].min());
	}

	unsigned int _three, aInd, bInd, cInd;
	for (int j = 0; j < numFaces; j++) {
		infile >> _three >> aInd >> bInd >> cInd;
		faces[j].a = &vertices[aInd];
		faces[j].b = &vertices[bInd];
		faces[j].c = &vertices[cInd];
	}
	normVerticesToRange(minVertex, maxVertex);
}

void normVerticesToRange(const float min, const float max) {
	float range = max - min;
	for (int i = 0; i < numVertices; i++) {
		vertices[i].x = -1 + ((vertices[i].x - min) * 2) / range;
		vertices[i].y = -1 + ((vertices[i].y - min) * 2) / range;
		vertices[i].z = -1 + ((vertices[i].z - min) * 2) / range;
	}
}
