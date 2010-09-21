void drawScene();
void initGraphics(int argc, char **argv);
void setCameraRotation(float angleX, float angleY);
void setCameraDist(float distance);
void setCameraPos(float x, float y, float z);
void setSafeShift(float x, float y, float z, float videoTick);
void deinitGraphics();

struct Vect {
	float x;
	float y;
	float z;
};

struct CameraShift {
	struct Vect p;
	struct Vect last;
	struct Vect v;
	//float t;
	int lastVideoTick;
	int lastGraphicTick;
	int graphicTickLength;
};
