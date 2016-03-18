#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#include <map>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <string>


#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "camera_secure.h"
#include "utils.h"

#define ROUND_UP_2(num)  (((num)+1)&~1)
#define ROUND_UP_4(num)  (((num)+3)&~3)
#define ROUND_UP_8(num)  (((num)+7)&~7)
#define ROUND_UP_16(num) (((num)+15)&~15)
#define ROUND_UP_32(num) (((num)+31)&~31)
#define ROUND_UP_64(num) (((num)+63)&~63)




#if 0
# define CHECK_REREAD
#endif

#if 1
# define FRAME_WIDTH  640
# define FRAME_HEIGHT 480
#else
# define FRAME_WIDTH  512
# define FRAME_HEIGHT 512
#endif

#if 0
# define FRAME_FORMAT V4L2_PIX_FMT_YUYV
#else
# define FRAME_FORMAT V4L2_PIX_FMT_YUYV
#endif

#define LOGO_KEY "VIDEO_LOGO"
#define CONFIG_FILE "/home/sysadmin/.camera_security"
#define PID_SAVE_FILE "/tmp/camera.pid"
#define VERSION_SAVE_FILE "/tmp/camera.version"
#define PID_SAVE_PATH "/tmp"
#define VIDEO_DEVICE "/dev/video1"
static int debug=0;
const char* face_secure_file = "/opt/camera_security/security_2.jpg";
const char* text_secure_file = "/opt/camera_security/security_3.jpg";
map<string, string> options;

const int MAX_STATUS_LASTING_COUNT = 3;
const float MAX_TEXT_COVER_AREA = 0.03f;
const int MAX_TEXT_COVER_COUNT = 7;


void parse(std::istream & cfgfile)
{
    for (string line; getline(cfgfile, line); )
    {
        istringstream iss(line);
        string id, eq, val;

        bool error = false;

        if (!(iss >> id))
        {
            error = true;
        }
        else if (id[0] == '#')
        {
            continue;
        }
        else if (!(iss >> eq >> val >> ws) || eq != "=" || iss.get() != EOF)
        {
            error = true;
        }

        if (error)
        {
            // do something appropriate: throw, skip, warn, etc.
        }
        else
        {
            options[id] = val;
        }
    }
}

void info_writing() {
	// write process ID
	_mkdir(PID_SAVE_PATH);
	FILE *fp;
	fp = fopen(PID_SAVE_FILE, "w");
	if (fp != NULL) {
		fprintf(fp, "%d\n", getpid());
	}
	fclose(fp);
	fp = fopen(VERSION_SAVE_FILE, "w");
	if (fp != NULL) {
		fprintf(fp, "%d.%d\n", VERSION, SUBVERSION);
	}
	fclose(fp);
	//
	ifstream f(CONFIG_FILE);
	parse(f);
	//printf("%s: %s\n", LOGO_KEY, options[LOGO_KEY].c_str());
}

int format_properties(const unsigned int format,
		const unsigned int width,
		const unsigned int height,
		size_t*linewidth,
		size_t*framewidth) {
size_t lw, fw;
	switch(format) {
	case V4L2_PIX_FMT_YUV420: case V4L2_PIX_FMT_YVU420:
		lw = width; /* ??? */
		fw = ROUND_UP_4 (width) * ROUND_UP_2 (height);
		fw += 2 * ((ROUND_UP_8 (width) / 2) * (ROUND_UP_2 (height) / 2));
	break;
	case V4L2_PIX_FMT_UYVY: case V4L2_PIX_FMT_Y41P: case V4L2_PIX_FMT_YUYV: case V4L2_PIX_FMT_YVYU:
		lw = (ROUND_UP_2 (width) * 2);
		fw = lw * height;
	break;
	default:
		return 0;
	}

	if(linewidth)*linewidth=lw;
	if(framewidth)*framewidth=fw;

	return 1;
}


void print_format(struct v4l2_format*vid_format) {
//  printf("	vid_format->type                =%d\n",	vid_format->type );
//  printf("	vid_format->fmt.pix.width       =%d\n",	vid_format->fmt.pix.width );
//  printf("	vid_format->fmt.pix.height      =%d\n",	vid_format->fmt.pix.height );
//  printf("	vid_format->fmt.pix.pixelformat =%d\n",	vid_format->fmt.pix.pixelformat);
//  printf("	vid_format->fmt.pix.sizeimage   =%d\n",	vid_format->fmt.pix.sizeimage );
//  printf("	vid_format->fmt.pix.field       =%d\n",	vid_format->fmt.pix.field );
//  printf("	vid_format->fmt.pix.bytesperline=%d\n",	vid_format->fmt.pix.bytesperline );
//  printf("	vid_format->fmt.pix.colorspace  =%d\n",	vid_format->fmt.pix.colorspace );
}

int main(int argc, char**argv)
{
	struct v4l2_capability vid_caps;
	struct v4l2_format vid_format;
	size_t framesize;
	size_t linewidth;
	const char*video_device = VIDEO_DEVICE;
	int fdwr = 0;
	int ret_code = 0;
	if (argc > 1) {
		video_device = argv[1];
	}
	fdwr = open(video_device, O_RDWR);
	assert(fdwr >= 0);
	ret_code = ioctl(fdwr, VIDIOC_QUERYCAP, &vid_caps);
	assert(ret_code != -1);

	memset(&vid_format, 0, sizeof(vid_format));
	ret_code = ioctl(fdwr, VIDIOC_G_FMT, &vid_format);
	if (debug)
		print_format(&vid_format);
	framesize = FRAME_WIDTH * FRAME_HEIGHT * 2;
	vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	vid_format.fmt.pix.width = FRAME_WIDTH;
	vid_format.fmt.pix.height = FRAME_HEIGHT;
	vid_format.fmt.pix.pixelformat = FRAME_FORMAT;
	vid_format.fmt.pix.sizeimage = framesize;
//	vid_format.fmt.pix.field = V4L2_FIELD_NONE;
//	vid_format.fmt.pix.bytesperline = linewidth;
	vid_format.fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;

	if (debug)
		print_format(&vid_format);
	ret_code = ioctl(fdwr, VIDIOC_S_FMT, &vid_format);
	assert(ret_code != -1);

	if (debug)
		print_format(&vid_format);

	if (!format_properties(vid_format.fmt.pix.pixelformat,
			vid_format.fmt.pix.width, vid_format.fmt.pix.height, &linewidth,
			&framesize)) {
//		printf("unable to guess correct settings for format '%d'\n",
//				FRAME_FORMAT);
	}

	info_writing();
	/*---------------------- opencv part --------------------------*/
	VideoCapture cap(0); //capture the video from web cam
	if (!cap.isOpened()) {
//		printf("Cannot open the web cam.\n");
		return -1;
	}

	Mat original, gray, text_secure_screen, face_secure_screen;
	face_secure_screen = imread(face_secure_file);
	text_secure_screen = imread(text_secure_file);
	int mCount = 0;
	SScreen status = NO_SECURE;
	SScreen t_status = NO_SECURE;
	void* yuyv;
	int i;
	while (true) {
		bool bSuccess = cap.read(original); // read a new frame from video
		if (!bSuccess) { //if not success, break loop
//			printf("Cannot read a frame from video stream");
			break;
		}
		//imshow("Original", original); //show the original image
		cvtColor(original, gray, CV_BGR2GRAY);

		IplImage* frame = cvCreateImage(cvSize(gray.cols, gray.rows), 8, gray.channels());
		IplImage tmp = gray;
		cvCopy(&tmp, frame);
		vector<Rect> faces = face_detect(frame);
		//text_detect(original);
		cvReleaseImage(&frame);
		//
		vector<Rect> texts = detectLetters2(original);
//		printf("Number of Text area: %d\n", texts.size());

		Size s = getTextSize(options[LOGO_KEY], CV_FONT_HERSHEY_SIMPLEX, 1.0, 1, NULL);

		if(faces.size() == 0){
			printf("No face!\n");
			mCount = (t_status == FACE_SECURE) ? (mCount+1):0;
			t_status = FACE_SECURE;
		} else {
			int area = 0;
			for (i = 0; i < texts.size(); i++) {
				area += texts[i].width * texts[i].height;
			}
			if (texts.size()>MAX_TEXT_COVER_COUNT || area>FRAME_WIDTH*FRAME_HEIGHT*MAX_TEXT_COVER_AREA) {
				printf("number of text region: %d\n", texts.size());
				printf("text area cover: %f%%\n", ((float) (area * 100)) / (FRAME_WIDTH * FRAME_HEIGHT));
				mCount = (t_status == TEXT_SECURE) ? (mCount + 1) : 0;
				t_status = TEXT_SECURE;
			} else {
				printf("number of text region: %d\n", texts.size());
				printf("text area cover: %f%%\n", ((float)(area*100))/(FRAME_WIDTH*FRAME_HEIGHT));
				mCount = (t_status == NO_SECURE) ? (mCount + 1) : 0;
				t_status = NO_SECURE;
			}
		}

		if(mCount>MAX_STATUS_LASTING_COUNT)
			status = t_status;
		if(status == NO_SECURE){
			rectangle(original, cvPoint(0,0), cvPoint(FRAME_WIDTH-1, FRAME_HEIGHT/8), CV_RGB(0,0,0), CV_FILLED);
			rectangle(original, cvPoint(0,FRAME_HEIGHT*7/8), cvPoint(FRAME_WIDTH-1, FRAME_HEIGHT-1), CV_RGB(0,0,0), CV_FILLED);
			rectangle(original, cvPoint(0,0), cvPoint(FRAME_WIDTH/4, FRAME_HEIGHT-1), CV_RGB(0,0,0), CV_FILLED);
			rectangle(original, cvPoint(FRAME_WIDTH*3/4,0), cvPoint(FRAME_WIDTH-1, FRAME_HEIGHT-1), CV_RGB(0,0,0), CV_FILLED);
			putText(original, options[LOGO_KEY], cvPoint(FRAME_WIDTH - 10 - s.width, FRAME_HEIGHT-10),
					CV_FONT_HERSHEY_SIMPLEX, 1.0, cvScalar(255,255,255), 1);
			yuyv = BGR2YUYV(original.data, FRAME_WIDTH, FRAME_HEIGHT);
		} else if(status == FACE_SECURE){
			putText(face_secure_screen, options[LOGO_KEY], cvPoint(FRAME_WIDTH - 10 - s.width, FRAME_HEIGHT-10),
					CV_FONT_HERSHEY_SIMPLEX, 1.0, cvScalar(255,255,255), 1);
			yuyv = BGR2YUYV(face_secure_screen.data, face_secure_screen.cols, face_secure_screen.rows);
		} else if(status == TEXT_SECURE){
			putText(text_secure_screen, options[LOGO_KEY], cvPoint(FRAME_WIDTH - 10 - s.width, FRAME_HEIGHT - 10),
					CV_FONT_HERSHEY_SIMPLEX, 1.0, cvScalar(255, 255, 255), 1);
			yuyv = BGR2YUYV(text_secure_screen.data, text_secure_screen.cols, text_secure_screen.rows);
		}
		if (framesize != write(fdwr, yuyv, framesize)) {
		}
		if (waitKey(30) == 27) {
			break;
		}
		free(yuyv);
	}
	close(fdwr);
	return 0;
}









