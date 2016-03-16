#ifndef _CAMERA_SECURE_H_
#define _CAMERA_SECURE_H_
#include <vector>
#include <opencv/cv.h>

using namespace cv;
using namespace std;

vector<Rect> face_detect(IplImage* input);
vector<Rect> detectLetters(Mat img);
vector<Rect> detectLetters2(Mat img);

#endif _CAMERA_SECURE_H_
