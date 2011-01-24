#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdio.h>
#include <ctype.h>

#define DEVIATION_THRESHOLD 70

IplImage *grey = 0, *prev_grey = 0, *pyramid = 0, *prevPyr = 0;

const int MAX_COUNT = 100;
int win_size = 10;
CvPoint2D32f* points[2] = {0,0}, *swapPoints;
CvPoint2D32f *validPoints;
CvPoint2D32f *vectors;
char* point_status = 0;
int count = 0;
int flags = 0;

int need_init = 1;

int init_opt_flow(IplImage *image) {
	grey = cvCreateImage(cvGetSize(image), 8, 1);
	prev_grey = cvCreateImage(cvGetSize(image), 8, 1);
	pyramid = cvCreateImage(cvGetSize(image), 8, 1);
	prevPyr = cvCreateImage(cvGetSize(image), 8, 1);
	points[0] = (CvPoint2D32f*)cvAlloc(MAX_COUNT * sizeof(*points[0]));
	points[1] = (CvPoint2D32f*)cvAlloc(MAX_COUNT * sizeof(*points[1]));
	point_status = (char*)cvAlloc(MAX_COUNT);

	validPoints = malloc(sizeof(*validPoints) * MAX_COUNT);
	vectors = malloc(sizeof(*vectors) * MAX_COUNT);

	count = MAX_COUNT;
}

// FIX freeing
void deinit_opt_flow() {
}

// Inverse sorting
int comparePointX(const void *arg1, const void *arg2) {
	return ((CvPoint2D32f *)arg2)->x - ((CvPoint2D32f *)arg1)->x;
}
int comparePointY(const void *arg1, const void *arg2) {
	return ((CvPoint2D32f *)arg2)->y - ((CvPoint2D32f *)arg1)->y;
}

int opt_flow_find_points(IplImage *prev_img, IplImage *image, int new_face_pos, CvPoint2D32f *pt1_in, CvPoint2D32f *pt2_in, CvPoint2D32f *pt1_out, CvPoint2D32f *pt2_out, IplImage *drawImage) {
	int i, j;
	char c;
	int square_len;

	float in_width = pt2_in->x - pt1_in->x;
	float in_height = pt2_in->y - pt1_in->y;

	if (prev_img == NULL || image == NULL)
		return -1;

	if (need_init) {
		/* allocate all the buffers */
		init_opt_flow(image);

		need_init = 0;
	}

	cvCvtColor(image, grey, CV_BGR2GRAY);
	cvCvtColor(prev_img, prev_grey, CV_BGR2GRAY);


	// Distribute input points evenly in face
	if (new_face_pos) {
		count = (in_width * in_height) / 50;
		if (count > MAX_COUNT)
			count = MAX_COUNT;

		for (i = 0; i < count; i++) {
			points[0][i].x = pt1_in->x + (float)rand()/RAND_MAX * in_width;
			points[0][i].y = pt1_in->y + (float)rand()/RAND_MAX * in_height;
		}

		/*square_len = (int)sqrt(count);
		printf("%f %f %d\n", in_width, in_height, square_len);
		for (j = 0; j < square_len; j++) {
			for (i = 0; i < square_len; i++) {
				points[0][i*(j+1)].x = pt1_in->x + (float)(i%square_len)/(square_len-1) * in_width;
				points[0][i*(j+1)].y = pt1_in->y + (float)j/(square_len-1) * in_height;
			}
		}*/
	}

	cvCalcOpticalFlowPyrLK(prev_grey, grey, prevPyr, pyramid,
	points[0], points[1], count, cvSize(win_size,win_size), 3, point_status, 0,
	cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), 0);

	//flags |= CV_LKFLOW_PYR_A_READY;


	for (i = 0, j = 0; i < count; i++) {
		if(!point_status[i])
			continue;

		validPoints[j] = points[1][i];
		vectors[j].x = points[1][i].x - points[0][i].x;
		vectors[j].y = points[1][i].y - points[0][i].y;

		j++;
	}


	// Find median velocity (vector)
	float medianVectorX, medianVectorY;

	qsort(vectors, j, sizeof(*vectors), comparePointX);
	medianVectorX = vectors[j/2].x;

	qsort(vectors, j, sizeof(*vectors), comparePointX);
	medianVectorY = vectors[j/2].y;

	//printf("Median vector: %f, %f\n", medianVectorX, medianVectorY);

	float medianVectorAngle = atan2f(medianVectorY, medianVectorX);
	float medianVectorValue = sqrtf(medianVectorX * medianVectorX + medianVectorY * medianVectorY);


	// Find median (center) point
	float centerX, centerY;

	qsort(validPoints, j, sizeof(*validPoints), comparePointX);
	centerX = validPoints[j/2].x;

	// Relocate points that deviate in position
	for (i = 0; i < j; i++) {
		if (fabs(validPoints[i].x - centerX) > DEVIATION_THRESHOLD) {
			validPoints[i].x = centerX + ((float)rand()/RAND_MAX - 0.5) * 100;
		}
	}

	qsort(validPoints, j, sizeof(*validPoints), comparePointY);
	centerY = validPoints[j/2].y;

	// Relocate points that deviate in position
	for (i = 0; i < j; i++) {
		if (fabs(validPoints[i].y - centerY) > DEVIATION_THRESHOLD) {
			validPoints[i].y = centerY + ((float)rand()/RAND_MAX - 0.5) * 100;
		}
	}



	/*if (fabs(vectors[j/5 - 1].x - medianVectorX) > DEVIATION_THRESHOLD || fabs(vectors[j/5 - 1].y - medianVectorY) > DEVIATION_THRESHOLD) {
		for (i = 0; i < j/5; i++) {
			validPoints[i].x = centerX + ((float)rand()/RAND_MAX - 0.5) * 100;
			validPoints[i].y = centerY + ((float)rand()/RAND_MAX - 0.5) * 100;
		}
	}*/

	for (i = 0; i < j; i++) {
		cvCircle(drawImage, cvPointFrom32f(validPoints[i]), 3, CV_RGB(0, 255, 0), -1, 8, 0);
	}

	if (i < 1)
		return -1;


	pt1_out->x = centerX - in_width/2;
	pt2_out->x = centerX + in_width/2;
	pt1_out->y = centerY - in_height/2;
	pt2_out->y = centerY + in_height/2;

	// Put the points back
	for (i = 0; i < j; i++) {
		points[1][i] = validPoints[i];
	}
	count = j;

	CV_SWAP(points[0], points[1], swapPoints);
	return 0;
}
