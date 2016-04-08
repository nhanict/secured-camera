#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>

#include <map>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <string>

// Network socket
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

// opencv
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "camera_secure.h"
#include "secure_camera_socket_handler.h"
#include "utils.h"
#include "image_processing.h"
//
#define ROUND_UP_2(num)  (((num)+1)&~1)
#define ROUND_UP_4(num)  (((num)+3)&~3)
#define ROUND_UP_8(num)  (((num)+7)&~7)
#define ROUND_UP_16(num) (((num)+15)&~15)
#define ROUND_UP_32(num) (((num)+31)&~31)
#define ROUND_UP_64(num) (((num)+63)&~63)

#define MAXPENDING 10    /* Max connection requests */
#define BUFFSIZE 32


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
#define CONFIG_FILE "/opt/camera_security/.camera_security"
#define PID_SAVE_FILE "/tmp/camera.pid"
#define VERSION_SAVE_FILE "/tmp/camera.version"
#define PID_SAVE_PATH "/tmp"
#define VIDEO_DEVICE "/dev/video1"
static int debug=0;
const char* face_secure_file = "/opt/camera_security/security_2.jpg";
const char* text_secure_file = "/opt/camera_security/security_3.jpg";
map<string, string> options;

const int MAX_STATUS_LASTING_COUNT = 1;
const int MAX_STATUS_LASTING_COUNT_FACE = 5;
const float MAX_TEXT_COVER_AREA = 0.01f;
const int MAX_TEXT_COVER_COUNT = 5;
SocketHandler *sh = NULL;


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
		fclose(fp);
	}
	fp = fopen(VERSION_SAVE_FILE, "w");
	if (fp != NULL) {
		fprintf(fp, "%d.%d\n", VERSION, SUBVERSION);
		fclose(fp);
	}
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
}

void *security_thread_execution(void *){
	//printf("\nI am in security thread");
	struct v4l2_capability vid_caps;
	struct v4l2_format vid_format;
	size_t framesize;
	size_t linewidth;
	const char*video_device = VIDEO_DEVICE;
	int fdwr = 0;
	int ret_code = 0;
//		if (argc > 1) {
//			video_device = argv[1];
//		}
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
	}

	info_writing();
	/*---------------------- opencv part --------------------------*/
	VideoCapture cap(0); //capture the video from web cam
	if (!cap.isOpened()) {
		//		printf("Cannot open the web cam.\n");
		return NULL;
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
		IplImage* frame = cvCreateImage(cvSize(gray.cols, gray.rows), 8,
				gray.channels());
		IplImage tmp = gray;
		cvCopy(&tmp, frame);
		vector<Rect> faces = face_detect(frame);
		//text_detect(original);
		cvReleaseImage(&frame);
		//
		vector<Rect> texts = detectLetters2(original);
		Size s = getTextSize(options[LOGO_KEY], CV_FONT_HERSHEY_SIMPLEX, 1.0, 1,
				NULL);

		int area = 0;
		for (i = 0; i < texts.size(); i++) {
			area += texts[i].width * texts[i].height;
		}
		if (texts.size() > MAX_TEXT_COVER_COUNT
				|| area > FRAME_WIDTH * FRAME_HEIGHT * MAX_TEXT_COVER_AREA) {
			mCount = (t_status == TEXT_SECURE) ? (mCount + 1) : 0;
			t_status = TEXT_SECURE;
		} else {
			//printf("faces number: %d\n", faces.size());
			if (faces.size() == 0) {
				mCount = (t_status == FACE_SECURE) ? (mCount + 1) : 0;
				t_status = FACE_SECURE;
			} else {
				mCount = (t_status == NO_SECURE) ? (mCount + 1) : 0;
				t_status = NO_SECURE;
			}
		}

		if (t_status == FACE_SECURE) {
			if (mCount > MAX_STATUS_LASTING_COUNT_FACE)
				status = t_status;
		} else {
			if (mCount > MAX_STATUS_LASTING_COUNT)
				status = t_status;
		}

		if (status == NO_SECURE) {
			rectangle(original, cvPoint(0, 0),
					cvPoint(FRAME_WIDTH - 1, FRAME_HEIGHT / 8), CV_RGB(0, 0, 0),
					CV_FILLED);
			rectangle(original, cvPoint(0, FRAME_HEIGHT * 7 / 8),
					cvPoint(FRAME_WIDTH - 1, FRAME_HEIGHT - 1), CV_RGB(0, 0, 0),
					CV_FILLED);
			rectangle(original, cvPoint(0, 0),
					cvPoint(FRAME_WIDTH / 4, FRAME_HEIGHT - 1), CV_RGB(0, 0, 0),
					CV_FILLED);
			rectangle(original, cvPoint(FRAME_WIDTH * 3 / 4, 0),
					cvPoint(FRAME_WIDTH - 1, FRAME_HEIGHT - 1), CV_RGB(0, 0, 0),
					CV_FILLED);
			putText(original, options[LOGO_KEY],
					cvPoint(FRAME_WIDTH - 10 - s.width, FRAME_HEIGHT - 10),
					CV_FONT_HERSHEY_SIMPLEX, 1.0, cvScalar(255, 255, 255), 1);
			yuyv = BGR2YUYV(original.data, FRAME_WIDTH, FRAME_HEIGHT);
		} else if (status == FACE_SECURE) {
			putText(face_secure_screen, options[LOGO_KEY],
					cvPoint(FRAME_WIDTH - 10 - s.width, FRAME_HEIGHT - 10),
					CV_FONT_HERSHEY_SIMPLEX, 1.0, cvScalar(255, 255, 255), 1);
			yuyv = BGR2YUYV(face_secure_screen.data, face_secure_screen.cols,
					face_secure_screen.rows);
		} else if (status == TEXT_SECURE) {
			putText(text_secure_screen, options[LOGO_KEY],
					cvPoint(FRAME_WIDTH - 10 - s.width, FRAME_HEIGHT - 10),
					CV_FONT_HERSHEY_SIMPLEX, 1.0, cvScalar(255, 255, 255), 1);
			yuyv = BGR2YUYV(text_secure_screen.data, text_secure_screen.cols,
					text_secure_screen.rows);
		}
		if (framesize != write(fdwr, yuyv, framesize)) {
		}
		if (waitKey(30) == 27) {
			break;
		}

		free(yuyv);
	}
	close(fdwr);
}

void *compose_msg(int &len){
	const char* HEAD = "CS";
	const char* TAIL = "TX";
	//const char* msg = "보안 스레드 상태";
	char* msg = "Receiving message confirmed!";
	int head_len = 10;
	int body_len = strlen(msg) + 2 + 2;
	len = head_len + body_len;
	void *out = malloc(len);
	memcpy(out, HEAD, 2);
	memcpy(out+2, &head_len, 4);
	memcpy(out+6, &body_len, 4);
	memcpy(out+head_len, &msg, strlen(msg));
	short stt = SECURE_RUNNING;
	memcpy(out+head_len+strlen(msg), &stt, 2);
	memcpy(out+head_len+strlen(msg) + 2, TAIL, 2);
	//return out;

	//
	len = strlen(msg);
	return msg;
}

void *thread_handle_client(void *sock_client){
	int write_len, read_len;
	void *write_msg, *read_msg;
	read_msg = malloc(MAX_SOCKET_MSG_LEN);
	write_msg = compose_msg(write_len);
	// read message from client
	while(1) {
		read_len = read(*(int *)sock_client, read_msg, MAX_SOCKET_MSG_LEN);
		//Send the message back to client
		if (read_len > 0) {
			printf("\nReceived message (%d): %s", *(int *)sock_client, read_msg);
			write(*(int *)sock_client, write_msg, write_len);
		} else if(read_len ==0 ){
			printf("\nDisconnected!");
			close(*(int *)sock_client);
			break;
		} else {
			printf("\nRead Failed!");
		}
		memset(read_msg, 0, MAX_SOCKET_MSG_LEN);
	}
}

void *socket_thread_execution(void *){
	int server_sock, client_sock;
	struct sockaddr_in server, client;
	if((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("\nFailed to open server socket!");
	}
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(SERVER_PORT);
	if(bind(server_sock, (struct sockaddr *) &server, sizeof(server))){
		perror("\nFailed to bind server socket.");
	}
	if(listen(server_sock, MAXPENDING) < 0){
		perror("\nFailed in listening client");
	}

	printf("\nOpen socker server %s in Port %d", SERVER_IP, SERVER_PORT);
	while(1){ /* Looping for listening client connections*/
		pthread_t th_client;
		int thread_num;
		unsigned int clientlen = sizeof(struct sockaddr_in);
		if((client_sock = accept(server_sock, (struct sockaddr *) &client, (socklen_t *)&clientlen)) < 0){
			perror("\nFailed to accept client socket.");
		}
		printf("\nConnected to client : %s Port %d", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		thread_num = pthread_create(&th_client, NULL, thread_handle_client, (void *) &client_sock);
	}
}


void signal_handler(int signal){
	printf( "\nsignal catched: %d\n", signal);
	if( remove(PID_SAVE_FILE) != 0 )
	    perror( "Error deleting file" );
	if(sh != NULL){
		sh->stop();
		delete sh;
	}
	exit(-1);
}

int main(int argc, char**argv)
{
	// signal handler
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGPIPE, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGKILL, signal_handler);
	signal(SIGCONT, signal_handler);
	signal(SIGHUP, signal_handler);
	signal(SIGABRT, signal_handler);
	signal(SIGSEGV, signal_handler);
	//

	int th_security_stt, th_socket_stt;
	pthread_t th_security, th_socket;
	if((th_security_stt = pthread_create(&th_security, NULL, security_thread_execution, NULL))){
		printf("\nError in create thread %d", th_security_stt);
	}

//	if((th_socket_stt = pthread_create(&th_socket, NULL, socket_thread_execution, NULL))){
//		printf("\nError in create thread %d", th_socket_stt);
//	}

	SocketHandler *sh = new SecureCameraSH();
	sh->start("", SERVER_PORT);

	pthread_join(th_security, NULL);
	//pthread_join(th_socket, NULL);

	return 0;
}









