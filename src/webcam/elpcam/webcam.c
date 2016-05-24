#include <opencv/highgui.h>
#include "log.h"
#include "webcam.h"

static CvCapture* cam;

int webcam_init(sConfigStruct * config, sWebCamStruct * camStruct)
{
	/* open camera */
	cam = cvCaptureFromCAM(CV_CAP_ANY);
	if (!cam)
	{
		log_error("couldn't open camera device");
		return -1;
	}

	/* setup x and y resolution */
	cvSetCaptureProperty(cam, CV_CAP_PROP_FRAME_WIDTH, config->webcam_xRes);
	cvSetCaptureProperty(cam, CV_CAP_PROP_FRAME_HEIGHT, config->webcam_yRes);

	camStruct->timestampBefore = malloc(sizeof(timeStruct));
	camStruct->timestampBefore = malloc(sizeof(timeStruct));

	return 0;
}

int webcam_get(sWebCamStruct * camStruct)
{
	IplImage *frame;
	int stat = 0;

	stat = getTime(camStruct->timestampBefore);
	if (!stat) {
		return -1;
	}

	/*download image from camera */
	frame = cvQueryFrame(cam);
	if (!frame) {
		log_error("couldn't get a frame");
		return -1;
	}

	/* put image information into structure */
	camStruct->buffer = frame->imageData;
	camStruct->bufferSize = frame->imageSize;

	return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int webcam_uninit(sConfigStruct * config)
{
	cvReleaseCapture(&cam);
	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

