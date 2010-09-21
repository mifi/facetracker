#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "SDL_opengl.h"



void myWireCube() {
	glBegin(GL_LINE_LOOP);
	glVertex3f(-1, -1, -1);
	glVertex3f(1, -1, -1);
	glVertex3f(1, -1, 1);
	glVertex3f(-1, -1, 1);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(-1, 1, -1);
	glVertex3f(1, 1, -1);
	glVertex3f(1, 1, 1);
	glVertex3f(-1, 1, 1);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(-1, -1, -1);
	glVertex3f(-1, 1, -1);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(1, -1, -1);
	glVertex3f(1, 1, -1);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(1, -1, 1);
	glVertex3f(1, 1, 1);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(-1, -1, 1);
	glVertex3f(-1, 1, 1);
	glEnd();
}

void drawTunnel() {
	int i;
	int n = 10;
	int m = 50;

	glColor3f(0.5f, 0.5f, 0.5f);

	for (i=0; i<m; i++) {
		float z = -(float)i;
		glBegin(GL_LINE_LOOP);
		glVertex3f(-1, -1, z);
		glVertex3f(1, -1, z);
		glVertex3f(1, 1, z);
		glVertex3f(-1, 1, z);
		glEnd();
	}

	for (i=0; i<n; i++) {
		float x = -1 + i*(float)2/n;
		glBegin(GL_LINES);
		glVertex3f(x, -1, 0);
		glVertex3f(x, -1, -m);
		glEnd();
		glBegin(GL_LINES);
		glVertex3f(x, 1, 0);
		glVertex3f(x, 1, -m);
		glEnd();

	}
	for (i=0; i<n; i++) {
		float y = -1 + i*(float)2/n;
		glBegin(GL_LINES);
		glVertex3f(-1, y, 0);
		glVertex3f(-1, y, -m);
		glEnd();
		glBegin(GL_LINES);
		glVertex3f(1, y, 0);
		glVertex3f(1, y, -m);
		glEnd();
	}
}

void fillCircle() {
	int i, num_lines = 50;

	glBegin(GL_POLYGON);
	for (i=0; i<num_lines; i++) {
		float angle = (float)i*2*M_PI/num_lines;
		glVertex2f(cos(angle), sin(angle));
	}
	glEnd();
}
void drawCircle() {
	int i, num_lines = 50;

	glBegin(GL_LINE_LOOP);
	for (i=0; i<num_lines; i++) {
		float angle = (float)i*2*M_PI/num_lines;
		glVertex2f(cos(angle), sin(angle));
	}
	glEnd();
}

void drawFlower(float distance, float x, float y) {
	// Circle fill
	glPushMatrix();
	glTranslatef(x, y, distance);
	glScalef(0.1, 0.1, 0.1);
	glColor3f(1, 0, 0);
	glLineWidth(5);
	fillCircle();

	// Line
	glColor3f(1, 1, 1);
	glLineWidth(2);
	glBegin(GL_LINES);
	glVertex3f(0, 0, -1);
	glVertex3f(-x, -y, -100);
	glEnd();
	glLineWidth(1);

	// Stroke
	/*glTranslatef(0.0, 0.0, 0.05);
	drawCircle();*/

	glColor3f(1, 1, 1);
	glTranslatef(0.0, 0.0, 0.1);
	glScalef(0.6, 0.6, 0.6);
	fillCircle();
	glPopMatrix();
}