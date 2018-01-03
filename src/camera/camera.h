/*
 * This implements the actual camera/framegrabber interface and will
 * mostly be a framegrabber SDK wrapper to allow for the undelying
 * SDK to be changed. This can also be used to mock the hardware
 * functions for unit/integration testing.
 * To support other framegrabbers, this file will have to be reimplemented.
 *
 *
 * A typical life cycle would be
 *
 * camera_init
 * camera_config
 * camera_setExposure
 *
 * camera_trigger
 * camera_get
 *
 * camera_abort
 * camera_uninit
 */
#ifndef _CAMERA_
#define _CAMERA_

/* Camera parameters */
typedef struct {
	/* A handle to identify the camera */
	unsigned long hCamera;

	/* Timestamp taken *before* the image has been triggered */
	timeStruct *timestampBefore;

	/* Timestamp taken after the image buffer has been returned */
	timeStruct *timestampAfter;

	/* contains the Exposuretime in [ms] */
	double dExposureTime;

	/* Pointer to image buffer */
	short * stBuffer;

	/* ~Callback stuff~ */

	/* Event Flags */
	volatile unsigned long fBufferReady;

	/* Control Flags */
	volatile unsigned long fFifoOverFlow;

	/* Camera identifier */
	char identifier;

	/* flag to indicate that the current image is a dark image */
	int dark;

} sParameterStruct;
#ifdef __cplusplus
extern "C" {
#endif

#include "configurations.h"

/**
 * inits the camera/framegrabber
 */
int camera_init(sParameterStruct *);

/**
 * stops (uninits) the camera/framegrabber and does neccesarry clean up
 */
int camera_uninit(sParameterStruct *);

/**
 * aquires one image/frame from the camera/framegrabber
 */
int camera_get(sParameterStruct *, int);

/**
 * abort the current aquisition
 */
int camera_abort(sParameterStruct *);

int camera_setExposure(sParameterStruct *);
int camera_autosetExposure(sParameterStruct *, sConfigStruct *);
int camera_config(sParameterStruct *);

#ifdef __cplusplus
}
#endif
#endif
