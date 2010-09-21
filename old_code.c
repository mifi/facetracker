
void processImage(IplImage *img) {
	int i, j;
	CvSize size = cvSize(img->width, img->height);

	float totalBrightness = 0;
	float avgBrightestPixel[2] = {0, 0};
	float hueLow = 0.68, hueHigh = 0.72;

	IplImage *hsv = cvCreateImage(size, IPL_DEPTH_8U, 3);

	cvCvtColor(img, hsv, CV_BGR2HSV);

	CvMat *mask = cvCreateMat(img->height, img->width, CV_8UC1);
	cvInRangeS(hsv, cvScalar(hueLow*256, 0.60*256, 0.20*256, 0),
		cvScalar(hueHigh*256, 1.00*256, 1.00*256, 0), mask);

	//IplImage *mask_img = cvCreateImage(size, 8, 1);
	//cvCopy(mask, mask_img, NULL);

	//cvCopy(mask_img, img, NULL);
	cvSet(img, cvScalar(255, 0, 0), mask);

	cvReleaseMat(&mask);

	cvReleaseImage(&hsv);


	for (i=0; i<img->width; i++) {
		for (j=0; j<img->height; j++) {
			/*rgb.r = ((uchar *)(img->imageData + i*img->widthStep))[j*img->nChannels + 2];
			rgb.g = ((uchar *)(img->imageData + i*img->widthStep))[j*img->nChannels + 1];
			rgb.b = ((uchar *)(img->imageData + i*img->widthStep))[j*img->nChannels + 0];*/

			//((uchar *)(img->imageData + i*img->widthStep))[j*img->nChannels + 0] = ((uchar *)(mask_img->imageData + i*mask_img->widthStep))[j*mask_img->nChannels];

/*			avgBrightestPixel[0] += i*hsv.val;
			avgBrightestPixel[1] += j*hsv.val;

			totalBrightness += hsv.val;*/
		}
	}

	//cvReleaseImage(&mask_img);


/*	avgBrightestPixel[0] /= totalBrightness;
	avgBrightestPixel[1] /= totalBrightness;

	if (avgBrightestPixel[0] == -1)
		return;

	CvPoint pt1, pt2;
	pt1.x = avgBrightestPixel[0] -6;
	pt1.y = avgBrightestPixel[0] -6;
	pt2.x = pt1.x + 6*2;
	pt2.y = pt1.y + 6*2;
	cvRectangle(img, pt1, pt2, CV_RGB(0, 0, 255), 3, 8, 0);*/
}

/*void processImage(IplImage *img) {
	int i, j;
	struct rgb_color rgb;
	struct hsv_color hsv;

	float totalBrightness = 0;
	float avgBrightestPixel[2] = {img->width/2, img->height/2};

	cvCvtColor(img, img, CV_BGR2HSV);


	//printf("%d %d %d %d %d %d %d\n%d\n", img->depth == IPL_DEPTH_8U, img->depth == IPL_DEPTH_8S, img->depth == IPL_DEPTH_16U, img->depth == IPL_DEPTH_16S, img->depth == IPL_DEPTH_32S, img->depth == IPL_DEPTH_32F, img->depth == IPL_DEPTH_64F, img->widthStep);

	for (i=0; i<img->width; i++) {
		for (j=0; j<img->height; j++) {
			rgb.r = ((uchar *)(img->imageData + i*img->widthStep))[j*img->nChannels + 2];
			rgb.g = ((uchar *)(img->imageData + i*img->widthStep))[j*img->nChannels + 1];
			rgb.b = ((uchar *)(img->imageData + i*img->widthStep))[j*img->nChannels + 0];

			hsv = rgb_to_hsv(rgb);

			avgBrightestPixel[0] += i*hsv.val;
			avgBrightestPixel[1] += j*hsv.val;

			totalBrightness += hsv.val;
		}
	}

	avgBrightestPixel[0] /= totalBrightness;
	avgBrightestPixel[1] /= totalBrightness;

	if (avgBrightestPixel[0] == -1)
		return;

	//printf("%f %f \n", avgBrightestPixel[0]/img->width, avgBrightestPixel[1]/img->height);

	CvPoint pt1, pt2;
	pt1.x = avgBrightestPixel[0] -6;
	pt1.y = avgBrightestPixel[0] -6;
	pt2.x = pt1.x + 6*2;
	pt2.y = pt1.y + 6*2;
	cvRectangle(img, pt1, pt2, CV_RGB(0, 0, 255), 3, 8, 0);
}*/
