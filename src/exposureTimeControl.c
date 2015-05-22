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

int setExposureTime(sParameterStruct * sSO2Parameters)
{
	int status = 0;		/* status variable */
	int timeSwitch = 0;	/* Integer switch to switch between exposure modi */
	tHandle hCamera = sSO2Parameters->hCamera;	/* hardware handle for camera */
	short *stBuffer;	/* Buffer where the Framegrabber stores the image */

	/* pre-set the buffer with zeros */
	memset(&stBuffer, 0, sizeof(stImageBuff));

	if (sSO2Parameters->dFixTime != 0) {
		/* Check if exposure time is declared fix in the config file if so set it. */
		logMessage("Set program to use a fix exposure time.");
		return camera_setExposure(sSO2Parameters);
	} else {
		/* Acquire first buffer to decide between FBL or SHT */
		status = camera_get(&sSO2Parameters, &stBuffer);
		if (status != 0) {
			return status;
		}

		/* calculate histogram to test for over or under exposition */
		evalHist(stBuffer, sSO2Parameters, &timeSwitch);

		camera_setExposureSwitch(sSO2Parameters, timeSwitch);
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
int evalHist(short *stBuffer, sParameterStruct * sSO2Parameters,
	int *timeSwitch)
{
	int bufferlength = sSO2Parameters->dBufferlength;
	int percentage = sSO2Parameters->dHistPercentage;
	int intervalMin = sSO2Parameters->dHistMinInterval;
	int histogram[4096] = { 0 };
	int summe = 0;
	int i;
	short temp = 0;
	short *shortBuffer;

	/* shortBuffer gets the address of the image data assigned
	 * since shortBuffer is of datatyp 'short'
	 * shortbuffer++ will set the pointer 16 bits forward
	 */
	shortBuffer = stBuffer;

	/* scanning the whole buffer and creating a histogram */
	for (i = 0; i < bufferlength; i++) {
		temp = *shortBuffer;
		histogram[temp]++;
		shortBuffer++;
	}

	/* sum over a through config file given interval to check if image is underexposed */
	for (i = 0; i < intervalMin; i++) {
		summe = summe + histogram[i];
	}

	/* pre-set the switch to 0 if image is neither over or under exposed it remains 0 */
	*timeSwitch = 0;

	/* check if the image is underexposed by testing if the sum of al values in a given interval
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
