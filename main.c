#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "SDL.h"
#include "SDL_thread.h"

#include "graphics.h"


//const char *cascade_path = "/usr/share/opencv/haarcascades/haarcascade_frontalface_alt.xml";
//const char *cascade_path = "/usr/share/opencv/haarcascades/haarcascade_mcs_eyepair_small.xml";
//const char *cascade_path = "/usr/share/opencv/haarcascades/haarcascade_mcs_upperbody.xml";
const char *cascade_path = "/usr/share/opencv/haarcascades/haarcascade_frontalface_alt2.xml";

static CvMemStorage* storage = NULL; // Create memory for calculations
static CvHaarClassifierCascade* cascade = 0; // Create a new Haar classifier

CvCapture* capture;

volatile static int quitFlag = 0;
volatile int videoTick = 0;


void faceDetect(IplImage *img) {
	int scale = 4;

	// Create a new image based on the input image
	IplImage *temp = cvCreateImage(cvSize(img->width/scale, img->height/scale), 8, 3);
	cvResize(img, temp, CV_INTER_LINEAR); //CV_INTER_NN

	// Create two points to represent the face locations
	CvPoint pt1, pt2;
	int i;

	cvClearMemStorage(storage);

	if (cascade) {
		// There can be more than one face in an image. So create a growable sequence of faces.
		// Detect the objects and store them in the sequence
		CvSeq* faces = cvHaarDetectObjects(temp, cascade, storage,
			1.1, 2, CV_HAAR_DO_CANNY_PRUNING,
			cvSize(40, 40));

		for(i = 0; i < (faces ? faces->total : 0); i++ ) {
			// Create a new rectangle for drawing the face
			CvRect *r = (CvRect*)cvGetSeqElem(faces, i);
			CvScalar color;

			// Find the dimensions of the face,and scale it if necessary
			pt1.x = r->x*scale;
			pt2.x = (r->x+r->width)*scale;
			pt1.y = r->y*scale;
			pt2.y = (r->y+r->height)*scale;

			if (i == 0)
				color = CV_RGB(255, 0, 0);
			else
				color = CV_RGB(0, 0, 255);

			// Draw the rectangle in the input image
			//cvRectangle(img, pt1, pt2, color, 3, 8, 0);
		}
        }
	cvReleaseImage(&temp);

	if (i > 0) {
		int centerX = pt1.x + (pt2.x - pt1.x) / 2;
		int centerY = pt1.y + (pt2.y - pt1.y) / 2;
		float xFraction = (float)centerX / img->width;
		float yFraction = (float)centerY / img->height;
		float zFraction = (float)(pt2.x - pt1.x) / img->width;
		float maxAngle = 20;

		setSafeShift((xFraction - 0.5) * 2, (yFraction - 0.5) * 2, zFraction, videoTick);
	}
}

int initFacedetect() {
	// Load the HaarClassifierCascade
	cascade = (CvHaarClassifierCascade *)cvLoad(cascade_path, 0, 0, 0 );

	// Check whether the cascade has loaded successfully. Else report and error and quit
	if( !cascade ) {
		fprintf( stderr, "ERROR: Could not load classifier cascade\n" );
		return -1;
	}

	// Allocate the memory storage
	storage = cvCreateMemStorage(0);

	return 0;
}


int videoFunc(void *unused) {
	IplImage* img = NULL; 

	while (!quitFlag) {
		img = cvQueryFrame(capture);
		//cvGrabFrame(capture);
		//img = cvRetrieveFrame(capture);
		if (img == NULL) {
			printf("Failed to grab frame\n");
			return -1;
		}


		IplImage *img2 = cvCloneImage(img);
		if (img2 == NULL) {
			printf("Failed to clone image\n");
			return -1;
		}

		faceDetect(img2);

		cvReleaseImage(&img2);

		videoTick++;
	}

	printf("Video thread exited\n");
	return 0;
}



void mainLoop() {
	int quit = 0;
	SDL_Event event;

	while (!quit) {
		drawScene();

		while(SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					quit = 1;
					break;
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_q)
						quit = 1;
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == SDL_BUTTON_RIGHT)
						quit = 1;
					break;
				default:
					break;
			}
		}
		SDL_Delay(10);
	}

	quitFlag = 1;
}

int main(int argc, char *argv[]) {
	if (initFacedetect() < 0)
		exit(-1);

	capture = cvCreateCameraCapture(-1);
	//capture = cvCaptureFromCAM(-1);
	if (capture == NULL) {
		printf("Couldn't start camera\n");
		exit(-1);
	}

	initGraphics(argc, argv);

	// Create video thread
	SDL_Thread *vidThread = SDL_CreateThread(&videoFunc, NULL);
	if (vidThread == NULL) {
		fprintf(stderr, "Unable to create thread: %s\n", SDL_GetError());
		return -1;
	}


	mainLoop();


	// Clean up
	SDL_WaitThread(vidThread, NULL);

	cvReleaseCapture(&capture);

	deinitGraphics();
	SDL_Quit();
	return 0;
}
