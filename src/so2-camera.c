#include<string.h>
#include<signal.h>
#include<stdlib.h>
#ifdef WIN
#include<windows.h>
#endif
#include "camera.h"
#include "configurations.h"
#include "imageCreation.h"
#include "filterwheel.h"
#include "log.h"
#include "io.h"
#include "webcam.h"
#include "spectrometer.h"
#include "spectroscopy.h"
#include "spectrometer-shutter.h"
#include "comm.h"
#include "threads.h"

/* explanation of prefixes:
 * d = Integer (int)
 * e = Status-Value (eStat) or Parameter-Value (etParamValue)
 * f = Flag (tFlag)
 * s = Structurs (struct)
 */

static sParameterStruct sParameters_A;
static sParameterStruct sParameters_B;

static sConfigStruct config;

static sWebCamStruct webcam;

static sSpectrometerStruct spectro;

static void stop_program(int reason);
static void emergency_stop_program(int reason);

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

	/* stop webcam */
	threads_webcam_stop();

	if (config.enableWebcam) {
		/* uninitialize webcam */
		webcam_uninit(&config, &webcam);
	}

	/* uninitialize io */
	io_uninit(&config);

	/* uninitialize filterwheel */
	filterwheel_uninit(&config);

	/* uninitialize spectrometer-shutter */
	spectrometer_shutter_uninit(&config);

	if (config.enableSpectroscopy) {
		/* stop spectrometer thread */
		threads_spectroscopy_stop();

		/* uninitialize spectrometer */
		spectrometer_uninit(&config);
	}

	/* stop communication and close all connections */
	comm_uninit(&config);

	/* stop logging and return file handle */
	log_uninit();

	printf("Program stopped. Bye!\n");

	/* now terminate process */
	_exit(reason);
}

/* Intercept SEGFAULT and try to at least kill all child processes.
 * Restarting the program won't work otherwise as long as those
 * processes are alive, since they are blocking comm->port.
 *
 * @author Pan Ick
 */
static void emergency_stop_program(int reason)
{
	comm_uninit(&config);
	_exit(reason);
}

/*
 * Windows control handlers, almost directly taken from
 * https://msdn.microsoft.com/en-us/library/ms685049%28VS.85%29.aspx
 */
#ifdef WIN
BOOL CtrlHandler(DWORD fdwCtrlType)
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

	/* embarrassingly there is a non-zero chance of this program
	 * segfaulting. Lets try to clean up enough so that the program can
	 * be restarted without manual intervention
	 */
	sa.sa_handler = &emergency_stop_program;
	sigaction(SIGSEGV, &sa, &osa);
#else
	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, (1 == 1))) {
		log_error("Control handler could not be installed, Ctrl+C won't work");
	}
#endif

	/*
	 * loading configuration is a three step process:
	 * 1- parsing command line arguments
	 * 2- parsing config file
	 * 3- loading default value
	 */

	/* init the config struct with empty values */
	config_init_sConfigStruct(&config);

	/*
	 * process the command line. Values specified on the cli take
	 * precedence over values defined elsewhere
	 */
	state = config_process_cli_arguments(argc, argv, &config);
	if (state != 0) {
		log_error("Could not handle command line arguments");
		stop_program(state);
	}

	/*
	 * load values from config values for properties that were not set
	 * on the command line
	 */
	state = config_load_configfile(&config);
	if (state != 0) {
		log_error("loading configuration failed");
		stop_program(state);
	}

	/*
	 * load default values for all properties that were not set on the
	 * cli or in the config file
	 */
	config_load_default(&config);

	/* initiate the logfile and start logging */
	state = log_init(&config);
	if (state != 0) {
		/*
		 * if creating a logfile fails we have to terminate the program.
		 * The error message then has to go directly to the screen
		 */
		printf("creating a logfile failed. Program is aborting...\n");
		return state;
	}

	/* Initialise parameter structures */
	config_init_sParameterStruct(&sParameters_A, &config, 'a');
	config_init_sParameterStruct(&sParameters_B, &config, 'b');

	/* initialize IO */
	state = io_init(&config);
	if (state != 0) {
		log_error("io_init failed");
		stop_program(1);
		return state;
	}

	/* init filterwheel */
	state = filterwheel_init(&config);
	if (state != 0) {
		log_error("failed to initialize filterwheel");
		stop_program(1);
		return state;
	}
	log_message("filterwheel initialized");

	/* open filterwheel */
	state = filterwheel_send(FILTERWHEEL_OPENED_A);
	if (state != 0) {
		log_error("failed to open filterwheel");
		stop_program(1);
		return state;
	}
	log_message("filterwheel opened");

	/* initiate communications */
	state = comm_init(&config);
	if (state != 0) {
		log_error("init comm failed");
		stop_program(1);
		return state;
	}

	if (config.enableWebcam) {
		/* initiate webcam */
		state = webcam_init(&config, &webcam);
		if (state != 0) {
			log_error("init webcam failed");
			stop_program(1);
			return state;
		}
		log_message("webcam initialized");

		/* start taking webcam images */
		threads_webcam_start(&config, &webcam);
	} else {
		log_message("disable webcam");
	}

	if (config.enableSpectroscopy) {
		/* initiate spectrometer-shutter */
		state = spectrometer_shutter_init(&config);
		if (state != 0) {
			log_error("init spectrometer-shutter failed");
			stop_program(1);
			return state;
		}
		log_message("spectrometer-shutter initialized");

		/* initiate spectrometer */
		state = spectrometer_init(&spectro);
		if (state != 0) {
			log_error("init spectrometer failed");
			stop_program(1);
			return state;
		}
		log_message("spectrometer initialized");

		state = spectroscopy_init(&config, &spectro);
		if (state != 0) {
			log_error("init spectroscopy failed");
			stop_program(1);
			return state;
		}
		log_message("spectroscopy initialized");

		/* start taking spectra */
		threads_spectroscopy_start(&config, &spectro);
	} else {
		log_message("disable spectroscopy");
	}

	/* initiate camera */
	state = camera_init(&sParameters_A);
	if (state != 0) {
		/* this is critical if this function fails no camera handle is returned */
		log_error("camera_init for Camera A failed");
		stop_program(1);
		return state;
	}
	log_message("camera A initialized");

	state = camera_init(&sParameters_B);
	if (state != 0) {
		/* this is critical if this function fails no camera handle is returned */
		log_error("camera_init for Camera B failed");
		stop_program(1);
		return state;
	}
	log_message("camera B initialized");

	/*
	 * Starting the acquisition with the exposure parameter set in
	 * configurations.c and exposureTimeControl.c
	 */
	state = startAquisition(&sParameters_A, &sParameters_B, &config);
	if (state != 0) {
		log_error("Aquisition failed");
		stop_program(1);
		return 1;
	}
	log_message("Aquisition stopped");

	/* we're done! */
	stop_program(0);

	return 0;
}
