#include<string.h> /* memset */
#include<stdlib.h>

#include "common.h"
#include "timehelpers.h"
#include "configurations.h"
#include "imageCreation.h"
#include "filterwheel.h"
#include "log.h"
#include "camera.h"
#include "spectrometer-shutter.h"
#include "spectrometer.h"
#include "spectroscopy.h"
#include "webcam.h"
#include "kbhit.h"
#include "io.h"
#include "exposureTimeControl.h"

int aquire_darkframe(sParameterStruct * sParameters_A, sParameterStruct * sParameters_B, sWebCamStruct * webcam, sSpectrometerStruct * spectro, sConfigStruct * config);

static void callback(sParameterStruct * sSO2Parameters);

static void callback(sParameterStruct * sSO2Parameters)
{
	sSO2Parameters->fBufferReady = TRUE;

	/* Increment the Display Buffer Ready Count */
	sSO2Parameters->dBufferReadyCount++;
}

int startAquisition(sParameterStruct * sParameters_A,
	sParameterStruct * sParameters_B, sWebCamStruct * webcam, sSpectrometerStruct * spectro, sConfigStruct * config)
{
	int i = 0, state = 0;
	log_message("Starting acquisition. Press a key to exit");

	for (i = 0; !kbhit() && (i < config->noofimages || config->noofimages == -1); i++) {
		if (i % config->darkframeintervall == 0){
			aquire_darkframe(sParameters_A, sParameters_B, webcam, spectro, config);
		}
		if (i % 1000 == 0){
			/* set exposure */
			state = setExposureTime(sParameters_A, config);
			if (state != 0) {
				log_error("setExposureTime for cam B failed");
				return 1;
			}
			log_message("exposure time for cam A set");

			state = setExposureTime(sParameters_B, config);
			if (state != 0) {
				log_error("setExposureTime for cam B failed");
				return 1;
			}
			log_message("exposure time for cam B set");
		}
		aquire(sParameters_A, sParameters_B, webcam, spectro, config);
	}

	return 0;
}

int aquire_darkframe(sParameterStruct * sParameters_A,
	sParameterStruct * sParameters_B, sWebCamStruct * webcam, sSpectrometerStruct * spectro, sConfigStruct * config)
{
	log_message("closing filterwheel");
	filterwheel_send(FILTERWHEEL_CLOSED_A);
	log_message("filterwheel closed");
	sParameters_A->dark = 1;
	sParameters_B->dark = 1;
	aquire(sParameters_A, sParameters_B, webcam, spectro, config);
	sParameters_A->dark = 0;
	sParameters_B->dark = 0;
	log_message("opening filterwheel");
	filterwheel_send(FILTERWHEEL_OPENED_A);
	log_message("filterwheel opened");


//	spectrometer_shutter_close();

//	spectroscopy_calibrate(spectro);

//	io_spectrum_save_calib(spectro, config);

//	spectrometer_shutter_open();

	return 0;
}

int aquire(sParameterStruct * sParameters_A, sParameterStruct * sParameters_B, sWebCamStruct * webcam, sSpectrometerStruct * spectro, sConfigStruct * config)
{
	int statusA = 0, statusB = 0, status = 0;

	/* get current time with milliseconds precision */
	getTime(sParameters_A->timestampBefore);
	getTime(sParameters_B->timestampBefore);

	if (statusA || statusB) {
		log_error("failed to get the timestampBefore time.");
		return 3;
	}

	/* Now start our capture, return control immediately back to program */
	statusA = camera_trigger(sParameters_A, callback);
	statusB = camera_trigger(sParameters_B, callback);

	if (statusA || statusB) {
		log_error("Starting the acquisition failed.");

		camera_abort(sParameters_A);
		camera_abort(sParameters_B);
		return 2;
	}


	/*
	 * Take webcam image and spectrometer measurement after the SO2
	 * images have been triggered. The assumption here is that the
	 * spectrum and webcam image take less time than we have to wait for
	 * the UV camera. This, however, might be wrong, and this algorithm
	 * will have to be changed to something smarter.
	 */


//	getTime(spectro->timestampBefore);
//	spectroscopy_measure(spectro);
//	getTime(spectro->timestampAfter);

	status = io_spectrum_save(spectro, config);
	if (status != 0) {
		log_error("failed to write spectrum");
		return status;
	}

	getTime(webcam->timestampBefore);
	status = webcam_get(webcam);
	getTime(webcam->timestampAfter);

	/* save webcam image */
	status = io_writeWebcam(webcam, config);
	if (status != 0) {
		log_error("failed to write webcam image");
		return status;
	}

	/* Wait for a user defined period between each camera trigger call */
	sleepMs(config->dInterFrameDelay);

	/* Wait here until either:
	 * (a) The user aborts the wait by pressing a key in the console window
	 * (b) The BufferReady event occurs indicating that the image is complete
	 * (c) The FIFO overflow event occurs indicating that the image is corrupt.
	 * Keep calling the sleep function to avoid burning CPU cycles */
	while (
		   !(sParameters_A->fBufferReady && sParameters_B->fBufferReady)
		&& !(sParameters_A->fFifoOverFlow && sParameters_B->fFifoOverFlow)
		&& !kbhit()
	){
		sleepMs(10);
	}

	/* Reset the buffer ready flags to false for next cycle */
	sParameters_A->fBufferReady = FALSE;
	sParameters_B->fBufferReady = FALSE;

	/* download the captured image */
	statusA = camera_get(sParameters_A);
	statusB = camera_get(sParameters_B);

	if (statusA || statusB) {
		log_error("Getting an image failed.");
		camera_abort(sParameters_A);
		camera_abort(sParameters_B);
		return 1;
	}

	/* get current time with milliseconds precision */
	getTime(sParameters_A->timestampAfter);
	getTime(sParameters_B->timestampAfter);

	/* save the captured image */
	statusA = io_write(sParameters_A, config);
	statusB = io_write(sParameters_B, config);

	if (statusA || statusB) {
		log_error("Saving an image failed.");
	}

	config->dImageCounter++;
	config->dImageCounter++;

	camera_abort(sParameters_A);
	camera_abort(sParameters_B);

	return statusA + statusB;
}
