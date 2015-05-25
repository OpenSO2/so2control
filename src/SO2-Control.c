#include<string.h>
#include<camera.h>
#include"configurations.h"
#include"imageCreation.h"
#include"exposureTimeControl.h"
#include"log.h"
/* explanation of prefixes:
 * d = Integer (int)
 * e = Status-Value (eStat) or Parameter-Value (etParamValue)
 * f = Flag (tFlag)
 * s = Structurs (struct)
 */

int main(int argc, char *argv[])
{
	/* definition of basic variables */
	sParameterStruct sParameters_A;
	sParameterStruct sParameters_B;
	int state;
	/*starting the logfile */
	state = log_init();
	if (state != 0) {
		/* if creating a logfile fails we have to terminate the program. The error message then has to go directly to the screen */
		log_error("creating a logfile failed. Program is aborting...\n");
		return state;
	}

	/* Initialise parameter structures */
	memset(&sParameters_A, 0, sizeof(sParameterStruct));
	memset(&sParameters_B, 0, sizeof(sParameterStruct));

	structInit(&sParameters_A, 'a');
	structInit(&sParameters_B, 'b');
	/* initiate camera */
	state = camera_init(&sParameters_A);
	if (state != 0) {
		/* this is critical if this function fails no camera handle is returned */
		log_error("camera_init for Camera A failed");
		return state;
	}

	state = camera_init(&sParameters_B);
	if (state != 0) {
		/* this is critical if this function fails no camera handle is returned */
		log_error("camera_init for Camera B failed");
		return state;
	}

	/* function for initialising basic values for sParameterStruct */
	state = configurations(&sParameters_A);
	state = configurations(&sParameters_B);
	if (state != 0) {
		log_error("configuration failed");
		return 1;
	}

	/* set exposure */
	setExposureTime(&sParameters_A);
	setExposureTime(&sParameters_B);

	/* Starting the acquisition with the exposure parameter set in configurations.c and exposureTimeControl.c */
	state = startAquisition(&sParameters_A, &sParameters_B);
	log_message("Aquisition stopped");
	if (state != 0) {
		log_error("Aquisition failed");
		return 1;
	}

	/* Now cease all captures */
	if (sParameters_A.hCamera)
		camera_uninit(&sParameters_B);
	if (sParameters_A.hCamera)
		camera_uninit(&sParameters_B);

	log_uninit();

	return 0;
}
