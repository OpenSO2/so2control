#include <opencv/highgui.h>
#include "log.h"
#include "webcam.h"

static CvCapture *cam;
long mean_exposure = 0;

int webcam_init(sConfigStruct * config, sWebCamStruct * webcam)
{
	int i;
	long noofmeanframes;
	long start = 0;
	/* open camera */
	cam = cvCaptureFromCAM(CV_CAP_ANY);
	if (!cam) {
		log_error("couldn't open webcam device");
		return -1;
	}

	/*
	 * Set webcam to the highest possible resolution by setting the
	 * resolution to some unreasonable high value which will be reduced
	 * to the max value by the device
	 */
	cvSetCaptureProperty(cam, CV_CAP_PROP_FRAME_WIDTH, 100000);
	cvSetCaptureProperty(cam, CV_CAP_PROP_FRAME_HEIGHT, 100000);
	webcam->width = cvGetCaptureProperty(cam, CV_CAP_PROP_FRAME_WIDTH);
	webcam->height = cvGetCaptureProperty(cam, CV_CAP_PROP_FRAME_HEIGHT);
	log_message("set webcam to resolution %i x %i", webcam->width, webcam->height);

	webcam->timestampBefore = (timeStruct *) malloc(sizeof(timeStruct));
	webcam->timestampAfter = (timeStruct *) malloc(sizeof(timeStruct));

	/*
	 * Unfortunately, both the camera device and the v4l driver have
	 * frame buffers and since we can't reliably flush these buffers, we
	 * are not guaranteed to receive the actual current scene. In my
	 * measurements, this amounted to sometimes more than 13s lag which
	 * is unacceptable. To work around the issue we can measure the time
	 * it takes to request a frame and compare that to the exposure time
	 * of the image. Annoyingly, opencv cannot determine the exposure
	 * time for v4l webcams. The workaround to this is to measure a mean
	 * return of several consecutive frames and use that as the norm to
	 * compare to.
	 */

	/* throw away the first few buffers to force a clean buffer */
	for (i = 0; i < 10; i++) {
		cvQueryFrame(cam);
	}

	/* use some frames to get a mean return time */
	noofmeanframes = 5;
	for (i = 0; i < noofmeanframes; i++) {
		start = getTimeStamp();
		cvQueryFrame(cam);
		mean_exposure += (getTimeStamp() - start) / noofmeanframes;
	}

	log_debug("webcam mean exposure is %lu", mean_exposure);

	return 0;
}

int webcam_get(sWebCamStruct * webcam)
{
	IplImage *frame;
	int stat = 0;
	long start, diff = 0;
	int i;

	/* if the time it takes to get a frame is a lot shorter than previously, there is something fishy going on; retry. */
	for (i = 0; i < 8 && diff < mean_exposure * .75; i++) {
		stat = getTime(webcam->timestampBefore);
		if (stat) {
			log_error("couldn't get the time before requesting a webcam frame\n");
			return -1;
		}

		/* download image from camera */
		start = getTimeStamp();
		frame = cvQueryFrame(cam);
		if (!frame) {
			log_error("couldn't get a webcam frame");
			return -2;
		}
		diff = getTimeStamp() - start;

		log_debug("time to get webcam image: %lu", diff);
	}
	log_debug("discarded %i webcam frames", i);

	stat = getTime(webcam->timestampAfter);
	if (stat) {
		log_error("couldn't get the time after requesting a webcam frame \n");
		return -1;
	}

	/* put image information into structure */
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
