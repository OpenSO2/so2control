#ifndef _CONFIGURATIONS_
#define _CONFIGURATIONS_

/**********************************************************
 *
 * Header-File with miscellaneous configuration functions
 * for Program, Camera and Framegrabber
 *
 **********************************************************/

/******************************
 *   HEADER INCLUDES
 ******************************/

#include<stdio.h>
#include<math.h>
#include "timehelpers.h"

/******************************
 *   Structures
 ******************************/

/* config structure */
typedef struct {
	/* flag. If trueish, processing is done */
	int processing;

	/* flag. If falseish, debugging output is suppressed */
	int debug;

	/* Number of images that are to be taken. Useful for testing */
	int noofimages;

	/* Number of pixels in 1 Image */
	int dBufferlength;

	/* exposure time in [us] */
	double dExposureTime_a;
	double dExposureTime_b;

	/* number of images taken */
	int dImageCounter;

	/* delay between two frames in [ms] */
	int dInterFrameDelay;

	/* contains the config filename */
	char * cConfigFileName;

	/* contains a prefix for all images */
	char * cFileNamePrefix;

	/* path to image directory */
	char * cImagePath;

	int createsubfolders;

	/*
	 * A switch to set the exposuretime fix to the value given in
	 * the config file
	 */
	int dFixTime;

	/*
	 * serial device used to talk to the filterwheel firmware
	 * eg. /dev/ttyUSB0
	 */
	char * filterwheel_device;

	/* number of images between */
	int darkframeintervall;

	/* serial device to talk to for the spectrometer shutter */
	const char * spectrometer_shutter_device;

	/*
	 * serial device channel on which the servo is connected (only used
	 * by the pololu maestro servo controller)
	 */
	unsigned char spectrometer_shutter_channel;

	/* measurement interval for recalibration of the doas instrument */
	int spectrometer_calibrate_intervall;

	/* region of interest for the spectrometer exposure time calculation */
	int spectroscopy_roi_upper;
	int spectroscopy_roi_lower;

	/* Resolution in x direction */
	int webcam_xRes;

	/* Resolution in y direction */
	int webcam_yRes;

	/* port on which a tcp connection can be made */
	int comm_port;

} sConfigStruct;


/*  */
typedef struct {
	double * wavelengths;

	double * lastSpectrum;

	double * electronic_offset;

	double * dark_current;

	double max;

	int integration_time_micros;

	int spectrum_length;

	timeStruct *timestampBefore;

	timeStruct *timestampAfter;
} sSpectrometerStruct;

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


/* Webcam parameters */
typedef struct {
	/* Pointer to image buffer x*y*24 RAW */
	char *buffer;

	/* Size of Buffer in bytes */
	int bufferSize;

	/* Timestamp taken *before* the image has been triggered */
	timeStruct *timestampBefore;

	/* Timestamp taken *after* the image has been received */
	timeStruct *timestampAfter;
} sWebCamStruct;


 /******************************
 *   FUNCTIONS
 ******************************/
int config_process_cli_arguments(int, char *[], sConfigStruct *);
int config_load_configfile(sConfigStruct *);
void config_load_default(sConfigStruct *);
void config_init_sParameterStruct(sParameterStruct *, sConfigStruct *, char);
void config_init_sConfigStruct(sConfigStruct *);
#endif
