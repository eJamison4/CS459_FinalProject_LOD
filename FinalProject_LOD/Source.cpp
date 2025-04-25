#include "Source.h"


int main(int argc, char* argv[]) {
	glutInit(&argc, argv);

	if (argc < 2) raise("Missing filepath argument...");

	glutInitWindowPosition(WINDOWPOS_X, WINDOWPOS_Y);
	glutInitWindowSize(WINDOWSIZE_X, WINDOWSIZE_Y);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	win = glutCreateWindow("LOD Algorithms Demo");
	OpenGLInit();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(resize);

	if (!init(argv[1])) return 1;

	glutMainLoop();
	free(faces);
	free(vertices);

	return 0;
}

bool init(const char filePath[]) {
	loadMeshFile_triangular(filePath);
	return true;
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	gluLookAt(
		0.0f, 0.0f, tz + 3.0f,
		0.0f, 0.4f, 0.0f,
		0.0f, 3.0f, 0.0f
	);

	glRotatef(xrot, 1.0f, 0.0f, 0.0f);
	glRotatef(yrot, 0.0f, 1.0f, 0.0f);

	drawMesh();
	drawLighting();

	glFlush();
	glutSwapBuffers();
}

void drawLighting() {
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambLight);
	glColor3f(1.0f, 1.0f, 0.0f);
	glDisable(GL_LIGHTING);
	glPushMatrix();
	glTranslatef(0.3f, 1.8f, 0.0f);
	glLightfv(GL_LIGHT0, GL_POSITION, light);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glPopMatrix();
}

void drawMesh() {
	glBegin(GL_TRIANGLES);
	triFace f;
	for (int i = 0; i < numFaces; i++) {
		f = faces[i];
		glVertex3f(f.a->x, f.a->y, f.a->z);
		glVertex3f(f.b->x, f.b->y, f.b->z);
		glVertex3f(f.c->x, f.c->y, f.c->z);
	}
	glEnd();
}

void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		mouseDepthMode = false;
		mouseDown = true;
		xdiff = x - yrot;
		ydiff = -y + xrot;
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		zdiff = -y + tz;
		mouseDepthMode = true;
		mouseDown = true;
	}
	else mouseDown = false;
}

void mouseMotion(int x, int y) {
	if (mouseDown) {
		if (mouseDepthMode) {
			tz += (ZFACT * (y - mousey));
		}
		else {
			xrot += (ANGFACT * (y - mousey));
			yrot += (ANGFACT * (x - mousex));
		}
		mousex = x;
		mousey = y;
		glutPostRedisplay();
	}
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'f':
	case 'F':
		decoupleCamDepth = true;
		break;
	case 'c':
	case 'C':
		decoupleCamDepth = false;
		break;
	case 27: // escape key
		exit(1);
	}
}

void resize(int w, int h) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glViewport(0, 0, w, h);

	gluPerspective(45.0f, 1.0f * w / h, 1.0f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
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
	float range = maxVertex - minVertex;
	for (int i = 0; i < numVertices; i++) {
		vertices[i].x = -1 + ((vertices[i].x - minVertex) * 2) / range;
		vertices[i].y = -1 + ((vertices[i].y - minVertex) * 2) / range;
		vertices[i].z = -1 + ((vertices[i].z - minVertex) * 2) / range;
	}
}

