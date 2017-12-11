/*
 * This file implements a simple "Expose to the Right" (ETTR) algorithm to
 * estimate a good (initial) exposure value. ETTR is a simple algorithm
 * suitable for most scenes, but can lead to a substantial underexposure
 * (leading decreased signal/noise ratio). However, a more sophisticated
 * algorithm could be implemented in eg. python and connected via the
 * socket interface. It might be necessary to manually adjust the
 * exposure, which can be done via the comm subsystem, the config file or
 * a command line option.
 *
 * For a nice overview of automatic exposure algorithms see eg.
 * Michael Muehlebach "Camera Auto Exposure Control for VSLAM Applications", Swiss Federal Institute of Technology, 2010
 */
#include<stdio.h>
#include<string.h>
#include "camera.h"
#include "configurations.h"
#include "imageCreation.h"
#include "log.h"

#define MAX_EXPOSURETIME 1004400

int find_ettr(double *, sParameterStruct *, sConfigStruct *);
int evalHist(sParameterStruct *, sConfigStruct *, int *);
double getExposureTime(sParameterStruct *, sConfigStruct *);

/* Either return the preset, fixed exposure time or run off into an
 * iterative loop to find a suitable value
 */
double getExposureTime(sParameterStruct * sSO2Parameters, sConfigStruct * config)
{
	double exposure;
	int status = 0;
	if (config->dFixTime != 0) {
		/* Check if exposure time is declared fix in the config file and if so, set it */
		log_message("Set program to use a fix exposure time, %i", config->dFixTime);
		if (sSO2Parameters->identifier == 'a')
			exposure = config->dExposureTime_a;
		else
			exposure = config->dExposureTime_b;
	} else {
		log_message("Find exposure");
		status = find_ettr(&exposure, sSO2Parameters, config);
		if (status) {
			log_error("could not find exposure (ETTR)");
			return status;
		}
	}

	sSO2Parameters->dExposureTime = exposure;

	return status;
}

int find_ettr(double *exposure, sParameterStruct * sSO2Parameters, sConfigStruct * config)
{
	int status = 0;
	int relative_exposure = 0;	/* 0 good exposure, -1 underexposure, 1 overexposure */
	double overexposed = -1;
	double underexposed = -1;
	char lastspeed[9];
	char speed[9];
	char lastm = '\0';
	char m;
	double actualExposure;
	*exposure = sSO2Parameters->dExposureTime;

	do {

		/* because the continues exposure time value maps to a finite set of
		 * discrete values (either frames in frameblanking mode or a number
		 * in the electronic shutter) there will be cases where the exposure
		 * time value changes, but the resulting calculated number does not
		 * which will lead to a pointless endless loop. So we check if the
		 * calculated number equals the last calculated number - which is
		 * then "good enough".
		 */
		calc_mode_speed(*exposure, &actualExposure, &m, speed);

		if (m == lastm && strncmp(speed, lastspeed, 9) == 0) {
			log_message("change in calculated exposure is lower than the amount that the camera can actually change by, so this is good enough.");
			/* unfortunately, this gets even more complicated
			 * If this image was overexposed, we are now stuck with blown
			 * out highlights, or in other words, loss of data. Unfortunately
			 * the brightest part of the sky coincides more often than not
			 * with the most interesting. So in that case we revert back
			 * to the last known underexposed value.
			 */
			if (relative_exposure == 1) {
				log_message("calculated value was overexposed, reverting to last known underexposed value");
				*exposure = underexposed;
			}
			break;
		}
		lastm = m;
		strncpy(lastspeed, speed, 9);

		sSO2Parameters->dExposureTime = *exposure;
		log_debug("find_ettr: camera_get %f", sSO2Parameters->dExposureTime);
		status = camera_setExposure(sSO2Parameters);
		if (status != 0) {
			log_error("unable to set exposure time");
			return status;
		}
		status = camera_get(sSO2Parameters, 1);
		if (status != 0) {
			log_error("could not get buffer for exposure control");
			return status;
		}

		evalHist(sSO2Parameters, config, &relative_exposure);
		/* do a bisect to find the optimal exposure time
		 * if overexposed, jump half the way to the last underexposed value
		 * if underexposed, jump half way to the last overexposed value
		 * if last over- or underexposed value is not yet known, half or double
		 * the current exposure time value
		 */
		if (relative_exposure == 1) {	/* overexposed */
			overexposed = *exposure;
			if (underexposed > 0) {
				*exposure = floor((*exposure + underexposed) / 2);
			} else {
				*exposure = floor(*exposure / 2);
			}

		} else if (relative_exposure == -1) {	/* underexposed */
			underexposed = *exposure;
			if (overexposed > 0) {
				*exposure = floor((*exposure + overexposed) / 2);
			} else if (abs(*exposure - MAX_EXPOSURETIME) < 2) {
				log_message("image is still underexposed, but exposure time has reached its max value (%i of %i)", floor(*exposure), MAX_EXPOSURETIME);
				relative_exposure = 0;
			} else {
				*exposure = (*exposure * 2 < MAX_EXPOSURETIME) ? *exposure * 2 : MAX_EXPOSURETIME;
			}
		}
		log_debug("relative_exposure %i %f", relative_exposure, *exposure);
	} while (relative_exposure);
	return 0;
}

/*
 * Create and inspect a histogram to determine if a image is under or over
 * exposed. This implements a simple ETTR algorithm (but without accepting
 * blown out highlights) by assuming that a correctly lit scene
 * - has at least some elements that are close to being saturated (>95%) and
 * - has no elements that are almost blown out (>99%).
 * Saturated elements are ignored, because they might just be dead.
 *
 * timeswitch = 0  exposed correctly
 *            > 0  overexposed
 *            < 0  underexposed
 *
*/
int evalHist(sParameterStruct * sSO2Parameters, sConfigStruct * config, int *timeswitch)
{
	int bufferlength = config->dBufferlength;
	int histogram[4096] = { 0 };
	int sum = 0;
	int i;
	short temp = 0;

	/* scan the whole buffer and create a histogram */
	for (i = 0; i < bufferlength; i++) {
		temp = sSO2Parameters->stBuffer[i];
		histogram[temp]++;
	}

	if (config->debug) {
		FILE *fd;
		char fname[1000];
		sprintf(fname, "exposurehist-%c-%f", sSO2Parameters->identifier, sSO2Parameters->dExposureTime);
		fd = fopen(fname, "w");
		for (i = 0; i < 4096; i++) {
			fprintf(fd, "%i %i\n", i, histogram[i]);
		}
		fclose(fd);

		sprintf(fname, "img-%c-%f", sSO2Parameters->identifier, sSO2Parameters->dExposureTime);
		fd = fopen(fname, "w");
		for (i = 0; i < bufferlength; i++) {
			fprintf(fd, "%i %i\n", i, sSO2Parameters->stBuffer[i]);
		}
		fclose(fd);
	}

	/* To implement ettr check that there are some pixels exposed to over
	 * 3900 (~95%) but less than 4050 (~99%) (excluding values > 4094 to
	 * ignore hot pixels)
	 */
	sum = 0;
	for (i = floor(4096 * .95); i < 4096 - 1; i++) {
		sum += histogram[i];
	}
	if (sum < bufferlength * .0001) {
		log_debug("image underexposed");
		*timeswitch = -1;
		return 0;
	}

	/* now check for overexposure */
	sum = 0;
	for (i = floor(4096 * .99); i < 4096 - 1; i++) {
		sum += histogram[i];
	}
	if (sum > bufferlength * .0001) {
		log_debug("image overexposed");
		*timeswitch = 1;
		return 0;
	}

	/* image seems to be correctly exposed, but lets do some sanity checks
	 * to at least print some warnings
	 */
	/* at least two thirds of the elements should be at least half full */
	for (i = floor(4096 * .75); i < 4096 - 1; i++) {
		sum += histogram[i];
	}
	if (sum < bufferlength / 2) {
		log_message("ETTR WARNING: scene might be poorly lit");
	}

	*timeswitch = 0;
	return 0;
}
