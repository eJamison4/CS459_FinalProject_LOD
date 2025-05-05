#include "Source.h"

void createMenu();
void loadOFFByIndex(int index);
void computePerlinVertexColors();

enum InteractionMode { MODE_NONE, MODE_ROTATE, MODE_ZOOM };
InteractionMode currentMode = MODE_ROTATE;  // default to rotate

const char* offFiles[] = {
	"de_aztec.off",
	"de_dust2.off",
	"de_office.off"
};

std::vector<point> vertexColors;

const int numOffFiles = sizeof(offFiles) / sizeof(offFiles[0]);
int main(int argc, char* argv[]) {
	glutInit(&argc, argv);

	if (argc < 2) raise("Missing filepath argument...");

	glutInitWindowPosition(WINDOWPOS_X, WINDOWPOS_Y);
	glutInitWindowSize(WINDOWSIZE_X, WINDOWSIZE_Y);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	win = glutCreateWindow("LOD Algorithms Demo");
	OpenGLInit();

	createMenu();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(resize);

	init(offFiles[0]);

	glutMainLoop();

	onExitFree();

	return 0;
}

void createMenu() {
	int menu = glutCreateMenu([](int index) {
		loadOFFByIndex(index);
		});

	for (int i = 0; i < numOffFiles; ++i) {
		glutAddMenuEntry(offFiles[i], i);
	}

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}
void loadOFFByIndex(int index) {
	if (index >= 0 && index < numOffFiles) {
		onExitFree(); // clean up existing model data

		tz = 0.0f;    // Reset zoom
		xrot = 0.0f;  // Reset rotation
		yrot = 0.0f;

		init(offFiles[index]);  // Load new model
		glutPostRedisplay();    // Redraw scene
	}
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
	computePerlinVertexColors();
	return true;
}


void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	if (decoupleCamDepth)
		gluLookAt(0.0f, 0.0f, lockedDepth + 3.0f, 0.0f, 0.9f, 0.0f, 0.0f, 3.0f, 0.0f);
	else
		gluLookAt(0.0f, 0.0f, tz + 3.0f, 0.0f, 0.2f, 0.0f, 0.0f, 3.0f, 0.0f);

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

void computePerlinVertexColors() {
	vertexColors.clear();
	vertexColors.resize(numVertices);

	float frequency = 1000.0f; // Controls Perlin scale

	for (int i = 0; i < numVertices; ++i) {
		point v = vertices[i];
		float noise = stb_perlin_noise3(v.x * frequency, v.y * frequency, v.z * frequency, 0, 0, 0);
		float intensity = 0.5f * noise + 0.5f; // Normalize to [0, 1]

		vertexColors[i].x = intensity;        // R
		vertexColors[i].y = intensity * 0.85f; // G
		vertexColors[i].z = intensity * 0.65f; // B
	}
}


void drawMesh() {
	glDisable(GL_LIGHTING); // Use color only
	glBegin(GL_TRIANGLES);
	if (lod_mode == 1)
		drawSimplified();
	else {
		for (unsigned int i = 0; i < indexCount; i++) {
			unsigned int index = vertexIndexBuffer[i];
			glColor3f(vertexColors[index].x, vertexColors[index].y, vertexColors[index].z);
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
		modifiedVertices = (point*)malloc(numVertices * sizeof(point));
		std::copy(&vertices[0], &vertices[numVertices], modifiedVertices);
		numModdedVertices = numVertices;
	}
	if (simplifiedIndexBuffer == NULL) {
		simplifiedIndexBuffer = (unsigned int*)malloc(indexCount * sizeof(unsigned int));
		std::copy(&vertexIndexBuffer[0], &vertexIndexBuffer[indexCount], simplifiedIndexBuffer);
		numSimpleIndices = indexCount;
	}

	if (manual_lod_depth != lastSampleDepthRatio) {
		sampleDebounce = NUMDRAWSUNTILNEXTSAMPLE;
		lastSampleDepthRatio = manual_lod_depth;
		handleMeshSimplify();
	}

	glBegin(GL_TRIANGLES);
	for (int i = 0; i < numSimpleIndices; i++) {
		int idx = simplifiedIndexBuffer[i];
		glColor3f(vertexColors[idx].x, vertexColors[idx].y, vertexColors[idx].z);
		glVertex3f(modifiedVertices[idx].x, modifiedVertices[idx].y, modifiedVertices[idx].z);
	}
	glEnd();
}


void mouse(int button, int state, int x, int y) {
	if (state == GLUT_DOWN) {
		mousex = x;
		mousey = y;
		mouseDown = true;
	}
	else {
		mouseDown = false;
	}
}


void mouseMotion(int x, int y) {
	if (!mouseDown) return;

	int dx = x - mousex;
	int dy = y - mousey;

	switch (currentMode) {
	case MODE_ZOOM:
		tz += (ZFACT * dy);
		std::cout << "Zoom: " << tz << "\n";
		break;

	case MODE_ROTATE:
		xrot += (ANGFACT * dy);
		yrot += (ANGFACT * dx);
		break;

	default:
		break;
	}

	mousex = x;
	mousey = y;
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case '1':
		lod_mode = NONE;
		break;
	case '2':
		lod_mode = DOWNSAMPLE;
		break;
	case 'f': case 'F':
		decoupleCamDepth = true;
		lockedDepth = tz;
		break;
	case 't': case 'T':
		decoupleCamDepth = false;
		break;
	case 'z': case 'Z':
		currentMode = MODE_ZOOM;
		std::cout << "Mode: Zoom\n";
		break;
	case 'r': case 'R':
		currentMode = MODE_ROTATE;
		std::cout << "Mode: Rotate\n";
		break;
	case ',':
		manual_lod_depth--;
		std::cout << "increase detail\n";
		break;
	case '.':
		manual_lod_depth++;
		std::cout << "decrease detail\n";
		break;
	case 27: // ESC
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
	if (line != "OFF") throw("Opened file is not an .OFF file...\n");

	meshopt_setAllocator(malloc, free);

	int _zero; // not used
	infile >> numOrigVertices >> numFaces >> _zero;

	originalVertices = (point*)malloc(numOrigVertices * sizeof(point));
	if (!originalVertices) throw("Failed mem alloc for vertices...\n");

	float ix, iy, iz;
	float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
	float maxX = -FLT_MAX, maxY = -FLT_MAX, maxZ = -FLT_MAX;
	float sumX = 0.0f, sumY = 0.0f, sumZ = 0.0f;

	for (int i = 0; i < numOrigVertices; i++) {
		infile >> ix >> iy >> iz;

		sumX += ix; sumY += iy; sumZ += iz;

		minX = std::min(minX, ix);
		minY = std::min(minY, iy);
		minZ = std::min(minZ, iz);

		maxX = std::max(maxX, ix);
		maxY = std::max(maxY, iy);
		maxZ = std::max(maxZ, iz);

		originalVertices[i].x = ix;
		originalVertices[i].y = iy;
		originalVertices[i].z = iz;
	}

	float centerX = sumX / numOrigVertices;
	float centerY = sumY / numOrigVertices;
	float centerZ = sumZ / numOrigVertices;

	float rangeX = maxX - minX;
	float rangeY = maxY - minY;
	float rangeZ = maxZ - minZ;
	float maxRange = std::max(rangeX, std::max(rangeY, rangeZ));

	for (int i = 0; i < numOrigVertices; i++) {
		originalVertices[i].x = 2.0f * (originalVertices[i].x - centerX) / maxRange;
		originalVertices[i].y = 2.0f * (originalVertices[i].y - centerY) / maxRange;
		originalVertices[i].z = 2.0f * (originalVertices[i].z - centerZ) / maxRange;
	}

	originalIndices = (unsigned int*)malloc((3 * numFaces) * sizeof(unsigned int));
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

	indexMesh(); // generate optimized buffers
}
