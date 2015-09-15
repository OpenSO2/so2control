#ifndef _CONFIGURATIONS_
#define _CONFIGURATIONS_

/**********************************************************
 *
 *	Header-File with miscellaneous configuration functions
 *	for Program, Camera and Framegrabber
 *
 **********************************************************/

/******************************
 *   HEADER INCLUDES
 ******************************/

#include<stdio.h>
#include<math.h>
#include "common.h"

/******************************
 *   MACROS
 ******************************/

#define MAX_STRING_LENGTH 256

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

	/* when controling the exposure time a histogram is made
	 * from 1 image. the lower part is evaluated for underexposure
	 * this give the evaluated invervall.
	 */
	int dHistMinInterval;

	/* when controling the exposure time a histogram is made
	 * from 1 image. This value gibs the percentage of pixels
	 * that are aloud to be under or overexposed.
	 */
	int dHistPercentage;

	/* length of the triggerpulse in [ms] */
	unsigned long dTriggerPulseWidth;

	/* contains the Exposuretime in [ms] */
	double dExposureTime;

	/* number of images taken */
	int dImageCounter;

	/* delay between two frames in [ms] */
	int dInterFrameDelay;

	/* contains the name of a Config-file */
	char cConfigFileName[MAX_STRING_LENGTH];

	/* contains a prefix for all images */
	char cFileNamePrefix[MAX_STRING_LENGTH];

	/* path to image diretory */
	char cImagePath[MAX_STRING_LENGTH];

	/*
	 * A switch to set the exposuretime fix to the value given in
	 * the config file
	 */
	int dFixTime;

	/*
	 * serial device used to talk to the filterwheel firmware
	 * eg. /dev/ttyUSB0
	 */
	char filterwheel_device[MAX_STRING_LENGTH];

	/* number of images between */
	int darkframeintervall;
} sConfigStruct;

/* Camera parameters */
typedef struct {
	/* not used right now. should contain a value for image noise */
	unsigned long dDarkCurrent;

	/* A handle to identify the camera */
	unsigned long hCamera;

	/* Timestamp taken *before* the image has been triggered */
	timeStruct *timestampBefore;

	/* length of the triggerpulse in [ms] */
	unsigned long dTriggerPulseWidth;

	/* contains the Exposuretime in [ms] */
	double dExposureTime;

	/* Pointer to image buffer */
	short *stBuffer;

	/* ~Callback stuff~ */

	/* Event Flags */
	volatile unsigned long fBufferReady;
	volatile int dBufferReadyCount;

	/* Control Flags */
	volatile unsigned long fFifoOverFlow;

	/* Camera identifier */
	char identifier;

	/* flag to indicate that the current image is a dark image */
	int dark;
} sParameterStruct;

 /******************************
 *   FUNCTIONS
 ******************************/
int structInit(sParameterStruct *sSO2Parameters, sConfigStruct *config, char identifier);
int process_cli_arguments(int argc, char *argv[], sConfigStruct *config);
int load_config(char *filename, sConfigStruct * config);
#endif
