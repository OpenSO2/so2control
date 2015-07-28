#include<string.h>
#include<signal.h>
#include<stdlib.h>
#ifdef WIN
#include<windows.h>
#endif
#include "camera.h"
#include "configurations.h"
#include "imageCreation.h"
#include "exposureTimeControl.h"
#include "log.h"
#include "io.h"

/* explanation of prefixes:
 * d = Integer (int)
 * e = Status-Value (eStat) or Parameter-Value (etParamValue)
 * f = Flag (tFlag)
 * s = Structurs (struct)
 */

static sParameterStruct sParameters_A;
static sParameterStruct sParameters_B;

static sConfigStruct config;

static void stop_program(int reason);

/* Stop programs and do general clean up
 *
 * @author Jan Itor
 */
static void stop_program(int reason)
{

	log_message("Stopping program...");

	/* Cease all captures */
	if (&sParameters_A.hCamera) {
		camera_abort(&sParameters_A);
		camera_uninit(&sParameters_A);
	}
	if (&sParameters_B.hCamera) {
		camera_abort(&sParameters_B);
		camera_uninit(&sParameters_B);
	}

	/* uninitialize io */
	io_uninit(&config);

	/* stop logging and return file handle */
	log_uninit();

	printf("Program stopped. Bye!\n");

	/* now terminate process */
	exit(reason);
}


/*
 * Windows control handlers, almost directly taken from
 * https://msdn.microsoft.com/en-us/library/ms685049%28VS.85%29.aspx
 */
#ifdef WIN
BOOL CtrlHandler( DWORD fdwCtrlType )
{
	if (fdwCtrlType == CTRL_C_EVENT)
		stop_program(1);
	return 0;
}
#endif

int main(int argc, char *argv[])
{
	/* definition of basic variables */
	int state;

	/* Handle signals. This is useful to intercept accidental Ctrl+C
	 * which would otherwise just kill the process without any cleanup.
	 * This could also be useful when the process is managed by some
	 * other program, e.g. systemD.
	 * - SIGINT will be send by pressing Ctrl+C
	 * - SIGTERM will be send by process managers, e.g. systemD
	 *   wishing to stop the process
	 *
	 * Windows has its own ctrl handlers, we only really care about
	 * Ctrl+C, but others could be implemented as well (see above).
	 */
#ifdef POSIX
	struct sigaction sa, osa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &stop_program;
	sigaction(SIGINT, &sa, &osa);
	sigaction(SIGTERM, &sa, &osa);
#else
	if( !SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, TRUE ) ){
		log_error("Control handler could not be installed, Ctrl+C won't work");
	}
#endif

	/* initiate the logfile and start logging */
	state = log_init();
	if (state != 0) {
		/*
		 * if creating a logfile fails we have to terminate the program.
		 * The error message then has to go directly to the screen
		 */
		printf("creating a logfile failed. Program is aborting...\n");
		stop_program(state);
		return state;
	}

	/*
	 * set default values, read config file and process cli arguments,
	 * which override settings in the config file
	 */
	config.dHistMinInterval = 350;
	config.dHistPercentage = 5;
	config.dInterFrameDelay = 10;
	config.dBufferlength = 1376256;

	state = load_config("configurations//SO2Config.conf", &config);
	if (state != 0) {
		log_error("loading configuration failed");
		stop_program(1);
		return 1;
	}

	state = process_cli_arguments(argc, argv, &config);
	if (state != 0) {
		log_error("Could not handle command line arguments");
		return state;
	}

	/* Initialise parameter structures */
	structInit(&sParameters_A, &config, 'a');
	structInit(&sParameters_B, &config, 'b');

	/* initialize IO */
	state = io_init(&config);
	if (state != 0) {
		log_error("io_init failed");
		stop_program(1);
		return state;
	}

	/* initiate camera */
	state = camera_init(&sParameters_A);
	if (state != 0) {
		/* this is critical if this function fails no camera handle is returned */
		log_error("camera_init for Camera A failed");
		stop_program(1);
		return state;
	}
	log_debug("camera A initialized");

	state = camera_init(&sParameters_B);
	if (state != 0) {
		/* this is critical if this function fails no camera handle is returned */
		log_error("camera_init for Camera B failed");
		stop_program(1);
		return state;
	}
	log_debug("camera B initialized");

	/* configure camera */
	state = camera_config(&sParameters_A);
	if (state != 0) {
		log_error("config camera A failed");
		stop_program(1);
		return 1;
	}
	log_debug("camera A configured");

	state = camera_config(&sParameters_B);
	if (state != 0) {
		log_error("config camera B failed");
		stop_program(1);
		return 1;
	}
	log_debug("camera B configured");

	/* set exposure */
	state = setExposureTime(&sParameters_A, &config);
	if (state != 0) {
		log_error("setExposureTime for cam B failed");
		stop_program(1);
		return 1;
	}
	log_debug("exposure time for cam A set");

	state = setExposureTime(&sParameters_B, &config);
	if (state != 0) {
		log_error("setExposureTime for cam B failed");
		stop_program(1);
		return 1;
	}
	log_debug("exposure time for cam B set");

	/*
	 * Starting the acquisition with the exposure parameter set in
	 * configurations.c and exposureTimeControl.c
	 */
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
