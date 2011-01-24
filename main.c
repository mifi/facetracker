#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "SDL.h"
#include "SDL_thread.h"

#include "main.h"
#include "opt_flow.h"
#include "options.h"


const char *cascade_path = CASCADE_PATH;


static CvMemStorage* storage = NULL; // Create memory for calculations
static CvHaarClassifierCascade* cascade = 0; // Create a new Haar classifier

CvCapture* capture;

static int quitFlag = 0;
int videoTick = 0;

int numImgs = 0;
char **imgPaths = NULL;

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

int faceDetect(IplImage *img, CvPoint2D32f *pt1, CvPoint2D32f *pt2) {
	int scale = SCALE_FACTOR;
	IplImage *temp;
	float closest = FLT_MAX;
	CvPoint closest_pt1, closest_pt2;
	CvRect *r;
	int pyramids = 1;

	if (pyramids)
		scale = 2;

	// Create a new image based on the input image, and optionally scale it down
	temp = cvCreateImage(cvSize(img->width/scale, img->height/scale), 8, 3);

	if (pyramids)
		cvPyrDown(img, temp, CV_GAUSSIAN_5x5);
	else
		cvResize(img, temp, CV_INTER_LINEAR); //CV_INTER_NN

	int i;

	cvClearMemStorage(storage);

	if (cascade) {
		// There can be more than one face in an image. So create a growable sequence of faces.
		// Detect the objects and store them in the sequence
		/*CvSeq* faces = cvHaarDetectObjects(temp, cascade, storage,
			1.1, 2, CV_HAAR_DO_CANNY_PRUNING,
			cvSize(40, 40));*/
		CvSeq* faces = cvHaarDetectObjects(temp, cascade, storage,
			1.1, 3, CV_HAAR_DO_CANNY_PRUNING,
			cvSize(15, 15));

		fprintf(stderr, "Found %d faces\n", faces->total);

		/*for(i = 0; i < (faces ? faces->total : 0); i++) {
			float dist;
			float c_x, c_y;
			float last_c_x, last_c_y;

			// Create a new rectangle for drawing the face
			CvRect *r = (CvRect*)cvGetSeqElem(faces, i);
			r->x *= scale; r->y *= scale; r->width *= scale; r->height *= scale;

			last_c_x = pt1->x + (pt2->x - pt1->x) / 2;
			last_c_y = pt1->y + (pt2->y - pt1->y) / 2;
			c_x = r->x + r->width / 2;
			c_y = r->y + r->height / 2;

			dist = sqrtf((c_x - last_c_x) * (c_x - last_c_x) + (c_y - last_c_y) * (c_y - last_c_y));


			if (dist < closest) {
				closest_pt1.x = r->x;
				closest_pt2.x = r->x + r->width;
				closest_pt1.y = r->y;
				closest_pt2.y = r->y + r->height;
			}
		}*/

		i = faces->total;

		if (i > 0) {
			// Create a new rectangle for drawing the face
			r = (CvRect*)cvGetSeqElem(faces, 0);
			r->x *= scale; r->y *= scale; r->width *= scale; r->height *= scale;

			closest_pt1.x = r->x;
			closest_pt2.x = r->x + r->width;
			closest_pt1.y = r->y;
			closest_pt2.y = r->y + r->height;
		}
        }
	cvReleaseImage(&temp);

	if (i > 0) {
		//printf("closest: (%d, %d), num: %d\n", closest_pt1.x, closest_pt1.y, i);
		pt1->x = closest_pt1.x;
		pt1->y = closest_pt1.y;
		pt2->x = closest_pt2.x;
		pt2->y = closest_pt2.y;

		return 0;
	}
	else
		return -1;
}


void draw_rect(IplImage *image, CvPoint2D32f pt1, CvPoint2D32f pt2, int red) {
	CvScalar color;
	CvPoint intPt1;
	CvPoint intPt2;

	intPt1.x = pt1.x;
	intPt1.y = pt1.y;
	intPt2.x = pt2.x;
	intPt2.y = pt2.y;

	color = red ? CV_RGB(255, 0, 0) : CV_RGB(0, 0, 255);
	cvRectangle(image, intPt1, intPt2, color, 3, 8, 0);
}

void drawFullscreenRect(IplImage *image) {
	CvScalar color;
	CvPoint pt1, pt2;

	color = CV_RGB(255, 255, 0);
	pt1.x = 4;
	pt1.y = 4;
	pt2.x = image->width - 4;
	pt2.y = image->height + 4;
	cvRectangle(image, pt1, pt2, color, 3, 8, 0);
}

int videoFunc() {
	IplImage *frame = NULL; 
	IplImage *image = NULL, *image2 = NULL;
	IplImage *prev_img = NULL;

	CvPoint2D32f facePt1, facePt2;

	int isTrackingFace = FALSE;
	int initOptFlow = FALSE;

	while (!quitFlag) {
		char c;

		// New detected face position
		CvPoint2D32f newFacePt1, newFacePt2;

		int detectedFace = FALSE;

		if (numImgs == 0) {
			frame = cvQueryFrame(capture);
			//cvGrabFrame(capture);
			//img = cvRetrieveFrame(capture);
			if (frame == NULL) {
				fprintf(stderr, "Failed to grab frame\n");
				return -1;
			}
		}
		else {
			frame = cvLoadImage(imgPaths[videoTick % numImgs], CV_LOAD_IMAGE_UNCHANGED);

			if (frame == NULL) {
				fprintf(stderr, "Failed to load input image\n");
				return -1;
			}
		}

		image = cvCloneImage(frame);
		image2 = cvCloneImage(frame);
		if (image == NULL || image2 == NULL) {
			fprintf(stderr, "Failed to clone image\n");
			return -1;
		}

		if (videoTick % 10 == 0 && faceDetect(image, &newFacePt1, &newFacePt2) > -1) {
		//if (videoTick == 0 && faceDetect(image, &newFacePt1, &newFacePt2) > -1) {
			//if (((float)(newFacePos.x - facePos.x) / image->width < FACE_MOVEMENT_THRESHOLD && (float)(newFacePos.y - facePos.y) / image->height < FACE_MOVEMENT_THRESHOLD)
			//	|| !isTrackingFace) {
				detectedFace = TRUE;
				initOptFlow = TRUE;
				facePt1 = newFacePt1;
				facePt2 = newFacePt2;
			//}
		}

		if (!detectedFace && prev_img != NULL) {
			fprintf(stderr, "Failed to detect face, trying to track\n");

			if (opt_flow_find_points(prev_img, image, initOptFlow, &facePt1, &facePt2, &newFacePt1, &newFacePt2, image2) < 0) {
				fprintf(stderr, "Warning: couldn't track any flow points\n");
				isTrackingFace = FALSE;
			}
			else {
				//if ((float)(newFacePos.x - facePos.x) / image->width < FACE_MOVEMENT_THRESHOLD && (float)(newFacePos.y - facePos.y) / image->height < FACE_MOVEMENT_THRESHOLD) {
					isTrackingFace = TRUE;
					facePt1 = newFacePt1;
					facePt2 = newFacePt2;
				/*}
				else {
					isTrackingFace = FALSE;
				}*/
			}

			if (initOptFlow)
				initOptFlow = FALSE;
		}
		else
			isTrackingFace = TRUE;



		// Set previous image pointer and free old previouse image
		cvReleaseImage(&prev_img);
		prev_img = image;


		// Draw the rectangle in the input image
		if (isTrackingFace)
			draw_rect(image2, facePt1, facePt2, detectedFace);
		else {
			drawFullscreenRect(image2);
		}

		cvFlip(image2, NULL, 1);
		cvShowImage("preview", image2);

		c = cvWaitKey(10);
		if(c == 27 || c == 'q')
			break;

		cvReleaseImage(&image2);

		if (numImgs > 0)
			cvReleaseImage(&frame);

		videoTick++;
	}

	return 0;
}

int main(int argc, char *argv[]) {
	// Face detection
	if (initFacedetect() < 0)
		exit(-1);

	numImgs = argc - 1;
	imgPaths = argv + 1;

	if (numImgs == 0) {
		// Camera input
		//capture = cvCaptureFromCAM(-1);
		capture = cvCreateCameraCapture(-1);
		if (capture == NULL) {
			printf("Couldn't start camera\n");
			exit(-1);
		}
	}

	cvNamedWindow("preview", 1);

	videoFunc();

	cvDestroyWindow("preview");

	if (numImgs == 0)
		cvReleaseCapture(&capture);

	deinit_opt_flow();

	SDL_Quit();
	return 0;
}
