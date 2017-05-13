#include <opencv/highgui.h>
#include "log.h"
#include "webcam.h"

static CvCapture *cam;

int webcam_init(sConfigStruct * config, sWebCamStruct * webcam)
{
	/* open camera */
	cam = cvCaptureFromCAM(CV_CAP_ANY);
	if (!cam) {
		log_error("couldn't open camera device");
		return -1;
	}

	webcam->timestampBefore = (timeStruct *) malloc(sizeof(timeStruct));
	webcam->timestampAfter = (timeStruct *) malloc(sizeof(timeStruct));

	return 0;
}

int webcam_get(sWebCamStruct * webcam)
{
	IplImage *frame;
	int stat = 0;

	stat = getTime(webcam->timestampBefore);
	if (stat) {
		printf("couldn't get the time before\n");
		return -1;
	}

	/*download image from camera */
	frame = cvQueryFrame(cam);
	if (!frame) {
		printf("couldn't get a frame");
		return -2;
	}

	stat = getTime(webcam->timestampAfter);
	if (stat) {
		printf("couldn't get the time after \n");
		return -1;
	}

	/* put image information into structure */
	webcam->width = frame->width;
	webcam->height = frame->height;
	webcam->buffer = frame->imageData;
	webcam->bufferSize = frame->imageSize;

	return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int webcam_uninit(sConfigStruct * config, sWebCamStruct * webcam)
{
	cvReleaseCapture(&cam);
	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

