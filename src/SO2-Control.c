#include<string.h>
#include<signal.h>
#include<stdlib.h>
#include "camera.h"
#include "configurations.h"
#include "imageCreation.h"
#include "exposureTimeControl.h"
#include "log.h"

/* explanation of prefixes:
 * d = Integer (int)
 * e = Status-Value (eStat) or Parameter-Value (etParamValue)
 * f = Flag (tFlag)
 * s = Structurs (struct)
 */

static sParameterStruct sParameters_A;
static sParameterStruct sParameters_B;


/* Stop programs and do general clean up
 *
 * @author Jan Itor
 */
int stop_program(int reason){

	log_message("Stopping program...");

	/* Cease all captures */
	if (&sParameters_A.hCamera){
		camera_abort(&sParameters_A);
		camera_uninit(&sParameters_A);
	}
	if (&sParameters_B.hCamera){
		camera_abort(&sParameters_B);
		camera_uninit(&sParameters_B);
	}

	/* stop logging and return file handle */
	log_uninit();

	printf("Program stopped. Bye!\n");

	/* now terminate process */
	exit(reason);
}

int main(int argc, char *argv[])
{
	/* definition of basic variables */
	int state;

	sConfigStruct config;

	/* Handle signals. This is useful to intercept accidental Ctrl+C
	 * which would otherwise just kill the process without any cleanup.
	 * This could also be useful when the process is managed by some
	 * other program, e.g. systemD.
	 * - SIGINT will be send by pressing Ctrl+C
	 * - SIGTERM will be send by process managers, e.g. systemD
	 *   wishing to stop the process
	 *
	 * Signals really only exist on posix (linux) systems, hence this
	 * functionality only works there.
	 */
	#ifdef POSIX
	struct sigaction sa, osa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &stop_program;
	sigaction(SIGINT, &sa, &osa);
	sigaction(SIGTERM, &sa, &osa);
	#endif

	/* initiate the logfile and start logging */
	state = log_init();
	if (state != 0) {
		/* if creating a logfile fails we have to terminate the program. The error message then has to go directly to the screen */
		log_error("creating a logfile failed. Program is aborting...\n");
		stop_program(state);
		return state;
	}

	state = process_cli_arguments(argc, argv, &config);
	if(state != 0){
			log_error("Could not handle command line arguments");
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
		stop_program(1);
		return state;
	}

	state = camera_init(&sParameters_B);
	if (state != 0) {
		/* this is critical if this function fails no camera handle is returned */
		log_error("camera_init for Camera B failed");
		stop_program(1);
		return state;
	}

	/* function for initialising basic values for sParameterStruct */
	state = configurations(&sParameters_A);
	state = configurations(&sParameters_B);
	if (state != 0) {
		log_error("configuration failed");
		stop_program(1);
		return 1;
	}

	/* set exposure */
	setExposureTime(&sParameters_A);
	setExposureTime(&sParameters_B);

	/* Starting the acquisition with the exposure parameter set in configurations.c and exposureTimeControl.c */
	state = startAquisition(&sParameters_A, &sParameters_B, &config);
	log_message("Aquisition stopped");
	if (state != 0) {
		log_error("Aquisition failed");
		stop_program(1);
		return 1;
	}

	/* we're done! */
	stop_program(0);

	return 0;
}
