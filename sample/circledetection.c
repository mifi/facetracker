#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

void detectCircles(IplImage *img) {
	CvSize size = cvSize(img->width, img->height);
	IplImage *hsv = cvCreateImage(size, IPL_DEPTH_8U, 3);

	float hueFrom = 0.68, hueTo = 0.72;

	cvCvtColor(img, hsv, CV_BGR2HSV);

	CvMat *mask = cvCreateMat(img->height, img->width, CV_8UC1);
	cvInRangeS(hsv, cvScalar(hueFrom*256, 0.60*256, 0.20*256, 0),
		cvScalar(hueTo*256, 1.00*256, 1.00*256, 0), mask);

	cvReleaseImage(&hsv);

	// Morphological operations
	IplConvKernel *se21 = cvCreateStructuringElementEx(21, 21, 10, 10, CV_SHAPE_RECT, NULL);
	IplConvKernel *se11 = cvCreateStructuringElementEx(11, 11, 5,  5,  CV_SHAPE_RECT, NULL);
	
	cvMorphologyEx(mask, mask, NULL, se21, CV_MOP_OPEN);
	cvMorphologyEx(mask, mask, NULL, se11, CV_MOP_CLOSE);
	//cvClose(mask, mask, se21); // See completed example for cvClose definition
	//cvOpen(mask, mask, se11);  // See completed example for cvOpen  definition
	cvReleaseStructuringElement(&se21);
	cvReleaseStructuringElement(&se11);

	/* Copy mask into a grayscale image */
	IplImage *hough_in = cvCreateImage(size, 8, 1);
	cvCopy(mask, hough_in, NULL);
	cvReleaseMat(&mask);
        cvSmooth(hough_in, hough_in, CV_GAUSSIAN, 15, 15, 0, 0);

	/* Run the Hough function */
	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *circles = cvHoughCircles(hough_in, storage,
		CV_HOUGH_GRADIENT, 4, size.height/10, 100, 40, 0, 0);
	cvReleaseMemStorage(&storage);

	cvReleaseImage(&hough_in);

	int i;
	for (i = 0; i < circles->total; i++) {
		float *p = (float*)cvGetSeqElem(circles, i);
		CvPoint center = cvPoint(cvRound(p[0]),cvRound(p[1]));
		CvScalar val = cvGet2D(mask, center.y, center.x);
		if (val.val[0] < 1) continue;
		cvCircle(img,  center, 3,             CV_RGB(0,255,0), -1, CV_AA, 0);
		cvCircle(img,  center, cvRound(p[2]), CV_RGB(255,0,0),  3, CV_AA, 0);
		cvCircle(mask, center, 3,             CV_RGB(0,255,0), -1, CV_AA, 0);
		cvCircle(mask, center, cvRound(p[2]), CV_RGB(255,0,0),  3, CV_AA, 0);
	}
}
