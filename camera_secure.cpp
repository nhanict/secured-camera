#include "camera_secure.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

/*
 * configure for face detection
 */
static int isInitiated = 0;
CvScalar RED = { 0, 0, 255 };
int scale_factor = 3;
int min_neighbors = 3;
int flags = CV_HAAR_DO_CANNY_PRUNING;
int min_size = 40;
int max_size = 120;
double scale = 2;
const char* cascade_name = "/opt/camera_security/haarcascade_frontalface_default.xml";
Mat original; //current frame
int keyboard; //input from keyboard
CvMemStorage* storage = NULL;
CvHaarClassifierCascade* cascade = NULL;


/**
 * Initiate face detection.
 */
void initiate(){
	storage = cvCreateMemStorage(0);
	cascade = (CvHaarClassifierCascade*)cvLoad(cascade_name, 0, 0, 0);
}

/**
 * Face detection.
 */
vector<Rect> face_detect(IplImage* input) {
	if (!isInitiated) {
		initiate();
		isInitiated = 1;
	}
	vector<Rect> myFaces;
	IplImage* frame = input;
	IplImage* small_img = cvCreateImage(
			cvSize(cvRound(frame->width / scale),
					cvRound(frame->height / scale)), 8, frame->nChannels);
	short i;
	cvResize(frame, small_img, CV_INTER_LINEAR);
	cvEqualizeHist(small_img, small_img);
	cvClearMemStorage(storage);
	if (cascade) {
		double t = (double) cvGetTickCount();
		CvSeq* faces = cvHaarDetectObjects(small_img, cascade, storage,
				1.05 + ((float) scale_factor) / 100, min_neighbors, flags,
				cvSize(min_size, min_size), cvSize(max_size, max_size));
		//1.1, 2, 0/*CV_HAAR_DO_CANNY_PRUNING*/, cvSize(30, 30) );
		t = (double) cvGetTickCount() - t;
		int max_area = 0;
		for (i = 0; i < (faces ? faces->total : 0); i++) {
			CvRect* r = (CvRect*) cvGetSeqElem(faces, i);
			if (r->width * r->height > max_area) {
				if (myFaces.size() > 0)
					myFaces.pop_back();
				myFaces.push_back(
						Rect(r->x * scale, r->y * scale, r->width * scale,
								r->height * scale));
			}
		}
	}
	cvReleaseImage(&small_img);
	return myFaces;
}

/**
 * Detect letter region
 */
vector<Rect> detectLetters(Mat img)
{
    vector<Rect> boundRect;
    Mat img_gray, img_sobel, img_threshold, element;
    cvtColor(img, img_gray, CV_BGR2GRAY);
    Sobel(img_gray, img_sobel, CV_8U, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT);
    threshold(img_sobel, img_threshold, 0, 255, CV_THRESH_OTSU+CV_THRESH_BINARY);
    element = getStructuringElement(cv::MORPH_RECT, cv::Size(30, 30) );
    morphologyEx(img_threshold, img_threshold, CV_MOP_CLOSE, element); //Does the trick
    vector<vector<Point> > contours;
    findContours(img_threshold, contours, 0, 1);
    vector<vector<Point> > contours_poly( contours.size() );
    for( int i = 0; i < contours.size(); i++ )
        if (contours[i].size()>100)
        {
            approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
            Rect appRect( boundingRect(Mat(contours_poly[i]) ));
            if (appRect.width>appRect.height)
                boundRect.push_back(appRect);
        }
    return boundRect;
}

/**
 * Detect letter region 2
 */
vector<Rect> detectLetters2(Mat img)
{
	vector<Rect> boundRect;
	Mat rgb;
	// downsample and use it for processing
	pyrDown(img, rgb);
	Mat small;
	cvtColor(img, small, CV_BGR2GRAY);
	// morphological gradient
	Mat grad;
	Mat morphKernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	morphologyEx(small, grad, MORPH_GRADIENT, morphKernel);
	// binarize
	Mat bw;
	threshold(grad, bw, 0.0, 255.0, THRESH_BINARY | THRESH_OTSU);
	// connect horizontally oriented regions
	Mat connected;
	morphKernel = getStructuringElement(MORPH_RECT, Size(1, 9));
	morphologyEx(bw, connected, MORPH_CLOSE, morphKernel);
	// find contours
	Mat mask = Mat::zeros(bw.size(), CV_8UC1);
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(connected, contours, hierarchy, CV_RETR_CCOMP,
			CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	// filter contours
	for (int idx = 0; idx >= 0; idx = hierarchy[idx][0]) {
		Rect rect = boundingRect(contours[idx]);
		Mat maskROI(mask, rect);
		maskROI = Scalar(0, 0, 0);
		// fill the contour
		drawContours(mask, contours, idx, Scalar(255, 255, 255), CV_FILLED);
		// ratio of non-zero pixels in the filled region
		double r = (double) countNonZero(maskROI) / (rect.width * rect.height);

		if (r > .45 /* assume at least 45% of the area is filled if it contains text */
		&& (rect.height > 10 && rect.width > 10) /* constraints on region size */
		/* these two conditions alone are not very robust. better to use something
		 like the number of significant peaks in a horizontal projection as a third condition */
		) {
			//rectangle(rgb, rect, Scalar(0, 255, 0), 2);
			boundRect.push_back(rect);
		}
	}
    return boundRect;
}
