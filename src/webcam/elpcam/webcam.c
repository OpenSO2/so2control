#include <stdio.h>
#include "webcam.h"
#include "configurations.h"
#include "timehelpers.h"

static CvCapture* cam;

int webcam_init(sConfigStruct * config)
{
	/* open camera */
	cam = cvCaptureFromCAM(CV_CAP_ANY);
	if ( !cam )
	{
		fprintf(stderr, "couldn't open Camera device\n");
		return -1;
	}
	/* setup x and y resolution */
	cvSetCaptureProperty(cam, CV_CAP_PROP_FRAME_WIDTH, config->webcam_xRes);
	cvSetCaptureProperty(cam, CV_CAP_PROP_FRAME_HEIGHT, config->webcam_yRes);

	return 0;
}

int webcam_get(sWebCamStruct * camStruct)
{
	IplImage *frame;
	int stat = 0;
	camStruct->timestampBefore = malloc(sizeof(timeStruct));
	stat = getTime(camStruct->timestampBefore);
	if (stat != 0)
	{
		return -1;
	}

	/*download image from camera */
	frame = cvQueryFrame(cam);
	if (frame == 0) {
		fprintf(stderr, "couldn't get a frame\n");
		return -1;
	}

	/* put image information into Strukture */
	camStruct->buffer = frame->imageData;
	camStruct->bufferSize = frame->imageSize;

	return 0;
}

int webcam_uninit(sConfigStruct * config)
{
	cvReleaseCapture( &cam );
	return 0;
}
