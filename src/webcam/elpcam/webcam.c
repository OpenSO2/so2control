#include <stdio.h>
#include "highgui.h"
#include "webcam.h"
#include "configurations.h"
#include "timehelpers.h"

static CvCapture* cam;

int webcam_init(sWebCamStruct *camStruct)
{
	/* open camera */
	cam = cvCaptureFromCAM(CV_CAP_ANY);
	    if ( !cam )
	    {
	        fprintf(stderr,"couldn't open Camera device\n");
	        return -1;
	    }
	    /* setup x and y resolution */
	    cvSetCaptureProperty( cam, CV_CAP_PROP_FRAME_WIDTH, camStruct->xRes);
	    cvSetCaptureProperty( cam, CV_CAP_PROP_FRAME_HEIGHT, camStruct->yRes );

	return 0;
}

int webcam_get(sWebCamStruct *camStruct)
{
	IplImage *frame;
	int stat = 0;
	stat = getTime(camStruct->timestampBefore);
	if (stat != 0)
	{
		return -1;
	}

	/*download image from camera */
	frame = cvQueryFrame(cam);
	if (frame == 0) {
		fprintf(stderr,"couldn't get a frame\n");
		return -1;
	}
	/* put image information into Strukture */
	camStruct->buffer = frame->imageData;
	camStruct->bufferSize = frame->imageSize;

	return 0;
}

int webcam_uninit(sWebCamStruct *camStruct)
{
    cvReleaseCapture( &cam );

	return 0;
}
