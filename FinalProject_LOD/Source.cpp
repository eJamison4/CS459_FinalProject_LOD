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

	onExitFree();

	return 0;
}

void onExitFree() {
	free(originalVertices);
	free(modifiedVertices);
	free(vertices);
	free(originalIndices);
	free(vertexIndexBuffer);
	free(simplifiedIndexBuffer);
}

bool init(const char filePath[]) {
	loadMeshFile_triangular(filePath);
	return true;
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	if (decoupleCamDepth)
		gluLookAt(
			0.0f, 0.0f, lockedDepth + 3.0f,
			0.0f, 0.9f, 0.0f,
			0.0f, 3.0f, 0.0f
		);
	else gluLookAt(
		0.0f, 0.0f, tz + 3.0f,
		0.0f, 0.2f, 0.0f,
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
	if (lod_mode == 1)
		drawSimplified();
	else {
		unsigned int index;
		for (unsigned int i = 0; i < indexCount; i++) {
			index = vertexIndexBuffer[i];
			glVertex3f(vertices[index].x, vertices[index].y, vertices[index].z);
		}
	}
	glEnd();
}

void handleMeshSimplify(){
	float threshold = 0.2f;
	size_t targetIndexCount = size_t(numSimpleIndices * threshold);
	float targetErr = 1e-1f;

	std::vector<unsigned int> lod(numSimpleIndices);
	float lodErr = 0.f;

	lod.resize(meshopt_simplifySloppy(
		&lod[0], &simplifiedIndexBuffer[0], numSimpleIndices, &modifiedVertices[0].x,
		numModdedVertices, 3 * sizeof(float), targetIndexCount, targetErr, &lodErr
	));
	std::copy(lod.begin(), lod.end(), simplifiedIndexBuffer);
	numSimpleIndices = lod.size();
}


void drawSimplified() {
	if (modifiedVertices == NULL) {
		modifiedVertices = (point*)malloc(numVertices * (3 * sizeof(float)));
		std::copy(&vertices[0], &vertices[numVertices], modifiedVertices);
		numModdedVertices = numVertices;
	}
	if (simplifiedIndexBuffer == NULL) {
		simplifiedIndexBuffer = (unsigned int*)malloc(indexCount * sizeof(unsigned int));
		std::copy(&vertexIndexBuffer[0], &vertexIndexBuffer[indexCount], simplifiedIndexBuffer);
		numSimpleIndices = indexCount;
	}
	// if (!sampleDebounce && (int(floor(tz / 4)) != lastSampleDepthRatio)) {
	if (manual_lod_depth != lastSampleDepthRatio) {
		sampleDebounce = NUMDRAWSUNTILNEXTSAMPLE;
		// lastSampleDepthRatio = int(floor(tz / 4));
		lastSampleDepthRatio = manual_lod_depth;
		handleMeshSimplify();
	}
	// sampleDebounce--;
	point v;
	if (modifiedVertices)
		for (int i = 0; i < numSimpleIndices; i++) {
			v = modifiedVertices[simplifiedIndexBuffer[i]];
			glVertex3f(v.x, v.y, v.z);
		}
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
			std::cout << "Depth: " << tz << "\n";
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
	case '1':
		lod_mode = NONE;
		break;
	case '2':
		lod_mode = DOWNSAMPLE;
		break;
	case 'f':
	case 'F':
		decoupleCamDepth = true;
		lockedDepth = tz;
		break;
	case 't':
	case 'T':
		decoupleCamDepth = false;
		break;
	case ',':
		manual_lod_depth--;
		std::cout << "increase detail\n";
		break;
	case '.':
		manual_lod_depth++;
		std::cout << "decrease detail\n";
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

void indexMesh() {
	indexCount = numIndices;


	std::vector<unsigned int>remap(indexCount); //TEMP
	std::fill(remap.begin(), remap.end(), ~0u);

	numVertices = meshopt_generateVertexRemap(&remap[0], &originalIndices[0], indexCount, &originalVertices[0], numOrigVertices, 3 * sizeof(float));

	vertexIndexBuffer = (unsigned int*)malloc(indexCount * sizeof(unsigned int));
	vertices = (point*)malloc(numVertices * 3 * sizeof(float));

	meshopt_remapIndexBuffer(vertexIndexBuffer, &originalIndices[0], indexCount, &remap[0]);

	meshopt_remapVertexBuffer(
		vertices, &originalVertices[0], numOrigVertices, sizeof(point), &remap[0]
	);
}

void loadMeshFile_triangular(const char filePath[]) {
	std::ifstream infile(filePath);
	std::string line;
	std::getline(infile, line);
	if ((line > "OFF") != 0) throw("Opened file is not an .OFF file...\n");
	meshopt_setAllocator(malloc, free);
	int _zero; // not used
	infile >> numOrigVertices >> numFaces >> _zero;

	originalVertices = (point*)malloc(numOrigVertices * (3 * sizeof(float)));
	if (!originalVertices) throw("Failed mem alloc for vertices...\n");

	float ix, iy, iz;
	for (int i = 0; i < numOrigVertices; i++) {
		infile >> ix >> iy >> iz;
		maxVertex = std::max(maxVertex, std::max(ix, std::max(iy, iz)));
		minVertex = std::min(minVertex, std::min(ix, std::min(iy, iz)));
		originalVertices[i].x = ix;
		originalVertices[i].y = iy;
		originalVertices[i].z = iz;
	}
	numVertices = numOrigVertices;
	float range = maxVertex - minVertex;
	for (int i = 0; i < numOrigVertices; i++) { // normalizing all points to btwn -1 and 1
		originalVertices[i].x = 2 * ((originalVertices[i].x - minVertex) / range) - 1;
		originalVertices[i].y = 2 * ((originalVertices[i].y - minVertex) / range) - 1;
		originalVertices[i].z = 2 * ((originalVertices[i].z - minVertex) / range) - 1;
	}

	originalIndices = (unsigned int*)malloc((3 * numFaces) * sizeof(UINT64));
	if (!originalIndices) throw("Not enough memory available to allocate face indices...\n");

	unsigned int _three, a, b, c;
	unsigned int buffIndex = 0;
	for (int j = 0; j < numFaces; j++) {
		infile >> _three >> a >> b >> c;
		originalIndices[buffIndex] = a;
		originalIndices[buffIndex + 1] = b;
		originalIndices[buffIndex + 2] = c;
		buffIndex += 3;
	}
	infile.close();
	numIndices = buffIndex;
	//origIndMap.reserve(origIndMap.size() + (numIndices / sizeof(int)));
	//std::copy(&originalIndices[0], &originalIndices[numIndices], std::back_inserter(origIndMap));
	indexMesh();
}
