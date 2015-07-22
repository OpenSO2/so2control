/*
 * Function for the automatic control of the exposure time.
 */
#include<stdio.h>
#include<string.h>
#include "camera.h"
#include "exposureTimeControl.h"
#include "configurations.h"
#include "imageCreation.h"
#include "log.h"

void cb(sParameterStruct * sSO2Parameters)
{
	sSO2Parameters->fBufferReady = TRUE;

	/* Increment the Display Buffer Ready Count */
	sSO2Parameters->dBufferReadyCount++;
}

int setExposureTime(sParameterStruct * sSO2Parameters, sConfigStruct * config)
{
	int status = 0;		/* status variable */
	int timeSwitch = 0;	/* Integer switch to switch between exposure modi */

	if (config->dFixTime != 0) {
		/* Check if exposure time is declared fix in the config file if so set it. */
		log_message("Set program to use a fix exposure time.");
		return camera_setExposure(sSO2Parameters, config);
	} else {
		/* Acquire first buffer to decide between FBL or SHT */
		camera_trigger(sSO2Parameters, cb);

		/* wait for camera aquisition */
		while (!sSO2Parameters->fBufferReady)
			sleepMs(10);

		sSO2Parameters->fBufferReady = FALSE;

		status = camera_get(sSO2Parameters);
		if (status != 0) {
			log_error("could not get buffer for exposure control");
			return status;
		}

		/* clean up */
		camera_abort(sSO2Parameters);

		/* calculate histogram to test for over or under exposition */
		evalHist(sSO2Parameters, config, &timeSwitch);

		camera_setExposureSwitch(sSO2Parameters, config, timeSwitch);
	}

	return 0;
}

/*
 *
 * timeSwitch:
 *    - 0 neither over- nor underexposed
 *    - 1 underexposed
 *    - 2 overexposed
 *    - 3 over- and underexposed
 *
 *
 * bufferlength = 1344 x 1024 number of pixels
 * Image date is stored in 12-bit data within 16-bit data
 * datatyp 'short' in Visual Studio v6.0 represents 16 bit in memory
 * this might be different on different compilers.
 * IF POSSIBLE CHANGE THIS TO SOMETHING LESS DIRTY
 */
int evalHist(sParameterStruct * sSO2Parameters, sConfigStruct * config,
	     int *timeSwitch)
{
	int bufferlength = config->dBufferlength;
	int percentage = config->dHistPercentage;
	int intervalMin = config->dHistMinInterval;
	int histogram[4096] = { 0 };
	int summe = 0;
	int i;
	short temp = 0;
	short *shortBuffer;

	/* shortBuffer gets the address of the image data assigned
	 * since shortBuffer is of datatyp 'short'
	 * shortbuffer++ will set the pointer 16 bits forward
	 */
	shortBuffer = sSO2Parameters->stBuffer;

	/* scanning the whole buffer and creating a histogram */
	for (i = 0; i < bufferlength; i++) {
		temp = shortBuffer[i];
		histogram[temp]++;
	}

	/* sum over a through config file given interval to check if image is underexposed */
	for (i = 0; i < intervalMin; i++) {
		summe = summe + histogram[i];
	}

	/* pre-set the switch to 0 if image is neither over or under exposed it remains 0 */
	*timeSwitch = 0;

	/* check if the image is underexposed by testing if the sum of all values in a given interval
	 * is greater than a given confidence value */
	if (summe > (bufferlength * percentage / 100)) {
		*timeSwitch = 1;
	}
	/* check if the image is overexposed by testing how often the brightest pixel appears in the image */
	if (histogram[4095] > (bufferlength * percentage / 100)) {
		if (*timeSwitch == 1) {
			/* If timeSwitch was already set to 1 the picture is underexposed and
			 * overexposed therefore the timeSwitch is set to 3
			 */
			*timeSwitch = 3;
		} else {
			/* Image is only overexposed */
			*timeSwitch = 2;
		}
	}
	return 0;
}
