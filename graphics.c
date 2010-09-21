#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>

#include "SDL.h"
#include "SDL_opengl.h"
#include "SDL_mutex.h"

#include "graphics.h"
#include "gl_shapes.h"


// http://www.orthostereo.com/geometryopengl.html

#define DTR 0.0174532925

struct camera {
	GLdouble leftfrustum;
	GLdouble rightfrustum;
	GLdouble bottomfrustum;
	GLdouble topfrustum;
	GLfloat modeltranslationX;
	GLfloat modeltranslationY;
	float zoom;
} camera;


int screenwidth = 1024;
int screenheight = 600;

float depthZ = -2.0; // depth of the object drawing

double fovy = 45; // field of view in y-axis
double aspect = 1;
double nearZ = 0.1; // near clipping plane
double farZ = 30.0; // far clipping plane


float cubeAngle = 0;

/* critical */
volatile struct CameraShift cameraShift = {0, 0, 0, 0, 0, 0};
int graphicTick = 0;
/* /critical */

struct Vect position = {0, 0, 0};

static SDL_mutex *mut;


struct CameraShift getSafeShift() {
	struct CameraShift safeShift;

	if (SDL_mutexP(mut) == -1) {
		fprintf(stderr, "Couldn't lock mutex\n");
		exit(-1);
	}

	safeShift = cameraShift;

	if (SDL_mutexV(mut) == -1) {
		fprintf(stderr, "Couldn't unlock mutex\n");
		exit(-1);
	}

	return safeShift;
}

void setSafeShift(float x, float y, float z, float videoTick) {
	if (SDL_mutexP(mut) == -1) {
		fprintf(stderr, "Couldn't lock mutex\n");
		exit(-1);
	}

	cameraShift.p.x = x;
	cameraShift.p.y = y;
	cameraShift.p.z = z;
	cameraShift.p.z = 0.3;


	if (graphicTick > 0 && videoTick > 0) {
		cameraShift.graphicTickLength = graphicTick - cameraShift.lastGraphicTick;
		cameraShift.v.x = (cameraShift.p.x - cameraShift.last.x) / cameraShift.graphicTickLength;
		cameraShift.v.y = (cameraShift.p.y - cameraShift.last.y) / cameraShift.graphicTickLength;
		cameraShift.v.z = (cameraShift.p.z - cameraShift.last.z) / cameraShift.graphicTickLength;
	}
	cameraShift.lastVideoTick = videoTick;
	cameraShift.lastGraphicTick = graphicTick;
	cameraShift.last = cameraShift.p;

	if (SDL_mutexV(mut) == -1) {
		fprintf(stderr, "Couldn't unlock mutex\n");
		exit(-1);
	}
}

void updateFrustum() {
	struct CameraShift safeShift = getSafeShift();
/*
	float x = safeShift.last.x + safeShift.v.x * (graphicTick - safeShift.lastGraphicTick);
	float y = safeShift.last.y + safeShift.v.y * (graphicTick - safeShift.lastGraphicTick);
	float z = safeShift.last.z + safeShift.v.z * (graphicTick - safeShift.lastGraphicTick);
*/

	if (safeShift.graphicTickLength > 0) {
		float factor = 2;
		position.x += (safeShift.p.x - position.x) * factor / safeShift.graphicTickLength;
		position.y += (safeShift.p.y - position.y) * factor / safeShift.graphicTickLength;
		position.z += (safeShift.p.z - position.z) * factor / safeShift.graphicTickLength;
	}

	float x = position.x;
	float y = position.y;
	float z = position.z;

	double screenZ = -depthZ;			//screen projection plane
	double top = nearZ*tan(DTR*fovy/2);		//sets top of frustum based on fovy and near clipping plane
	double right = aspect*top;			//sets right of frustum based on aspect ratio
	double frustumshiftX = (x/2)*nearZ/screenZ;
	double frustumshiftY = (y/2)*nearZ/screenZ;

	camera.topfrustum = top + frustumshiftY;
	camera.bottomfrustum = -top + frustumshiftY;
	camera.leftfrustum = -right + frustumshiftX;
	camera.rightfrustum = right + frustumshiftX;
	camera.modeltranslationX = x/2;
	camera.modeltranslationY = y/2;

	camera.zoom = z;
}

void reshape(int w, int h) {
	if (h == 0)
		h = 1; // prevent divide by 0

	aspect = (double)w/h;
	glViewport(0, 0, w, h);
}

void drawScene() {
	updateFrustum();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(camera.leftfrustum, camera.rightfrustum, // set left view frustum
		camera.bottomfrustum, camera.topfrustum,
		nearZ, farZ);

	glTranslatef(camera.modeltranslationX, camera.modeltranslationY, 0.0); // translate to cancel parallax
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushMatrix();

	glTranslatef(0.0, 0.0, depthZ); // translate to screenplane

	// Tunnel
	glPushMatrix();
	glScalef(aspect, 1, 1);
	drawTunnel();
	glPopMatrix();


	glTranslatef(0.0, 0.0, camera.zoom*3-1);

	// Cube
	glPushMatrix();
	glTranslatef(-0.8, 0.2, -0.3);
	glScalef(0.25f, 0.25f, 0.25f);
	glLineWidth(10);
	glRotatef(cubeAngle, 1, 1, 0);
	glRotatef(-cubeAngle, 0, 0, 1);
	glColor3f(1, 1, 0);
	myWireCube();
	glLineWidth(1);
	glPopMatrix();
	cubeAngle -= 1.5;

	// Flowers
	drawFlower(1.5, 0.2, 0);
	drawFlower(1.0, -0.2, -0.1);
	drawFlower(0.5, 0, -0.2);
	drawFlower(-1, -0.1, 0.2);
	drawFlower(-10, 0.1, 0.2);


	glPopMatrix();
	SDL_GL_SwapBuffers();

	SDL_mutexP(mut);
	graphicTick++;
	SDL_mutexV(mut);
}

void deinitGraphics() {
	SDL_DestroyMutex(mut);
}

void initGraphics(int argc, char **argv) {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return;
	}

	mut = SDL_CreateMutex();

	GLfloat fogColor[] = {0, 0, 0, 1};

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_Surface *screen = SDL_SetVideoMode(0, 0, 0, SDL_OPENGL);// | SDL_FULLSCREEN);
	if (!screen) {
		return;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);

	// Fog
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_DENSITY, 0.35f);
	glHint(GL_FOG_HINT, GL_DONT_CARE);
	glFogf(GL_FOG_START, 5);
	glFogf(GL_FOG_END, 30);
	glEnable(GL_FOG);

	glClearColor(0, 0, 0, 1);


	reshape(screenwidth, screenheight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
