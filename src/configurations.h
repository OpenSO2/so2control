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
#include "common.h"
#include "timehelpers.h"

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
	 * from 1 image. The lower part is evaluated for underexposure
	 * this gives the evaluated interval.
	 */
	int dHistMinInterval;

	/* when controlling the exposure time a histogram is made
	 * from 1 image. This value is the percentage of pixels
	 * that are allowed to be under- or overexposed.
	 */
	int dHistPercentage;

	/* length of the triggerpulse in [ms] */
	unsigned long dTriggerPulseWidth;

	/* exposure time in [ms] */
	double dExposureTime;

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
} sConfigStruct;


/*  */
typedef struct {
	double * wavelengths;

	double * lastSpectrum;

	double * electronic_offset;

	double * dark_current;

	int integration_time_micros;

	int spectrum_length;
} sSpectrometerStruct;

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
	short * stBuffer;

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
int config_process_cli_arguments(int argc, char *argv[], sConfigStruct *config);
int config_load_configfile(sConfigStruct *config);
void config_load_default(sConfigStruct *config);
void config_init_sParameterStruct(sParameterStruct *sSO2Parameters, sConfigStruct *config, char identifier);
void config_init_sConfigStruct(sConfigStruct *config);
#endif
