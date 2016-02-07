#ifdef WIN
#include<windows.h>
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include<string.h> /* memset */
#include<stdlib.h>

#include<time.h>
#include "common.h"
#include "configurations.h"
#include "imageCreation.h"
#include "filterwheel.h"
#include "log.h"
#include "camera.h"
#include "kbhit.h"
#include "io/io.h"

static void callback(sParameterStruct * sSO2Parameters);

static void callback(sParameterStruct * sSO2Parameters)
{
	sSO2Parameters->fBufferReady = TRUE;

	/* Increment the Display Buffer Ready Count */
	sSO2Parameters->dBufferReadyCount++;
}

static int aquire_darkframe(sParameterStruct * sParameters_A,
	sParameterStruct * sParameters_B, sConfigStruct * config)
{
	log_message("closing filterwheel");
	filterwheel_send(FILTERWHEEL_CLOSED_A);
	log_message("filterwheel closed");
	sParameters_A->dark = 1;
	sParameters_B->dark = 1;
	aquire(sParameters_A, sParameters_B, config);
	sParameters_A->dark = 0;
	sParameters_B->dark = 0;
	log_message("opening filterwheel");
	filterwheel_send(FILTERWHEEL_OPENED_A);
	log_message("filterwheel opened");
	return 0;
}

int startAquisition(sParameterStruct * sParameters_A,
	sParameterStruct * sParameters_B, sConfigStruct * config)
{
	int i = 0;
	log_message("Starting acquisition...\n");
	log_message("Press a key to exit\n");

	for (i = 0; !kbhit() && (i < config->noofimages || config->noofimages == -1); i++) {
		if (i % config->darkframeintervall == 0){
			aquire_darkframe(sParameters_A, sParameters_B, config);
		}
		aquire(sParameters_A, sParameters_B, config);
	}

	return 0;
}

int aquire(sParameterStruct * sParameters_A, sParameterStruct * sParameters_B, sConfigStruct * config)
{
	int statusA = 0, statusB = 0;

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

time_t TimeFromTimeStruct(const timeStruct * pTime)
{
	struct tm tm;
	memset(&tm, 0, sizeof(tm));

	tm.tm_year = pTime->year - 1900;
	tm.tm_mon = pTime->mon - 1;
	tm.tm_mday = pTime->day;
	tm.tm_hour = pTime->hour;
	tm.tm_min = pTime->min;
	tm.tm_sec = pTime->sec;

	return mktime(&tm);
}

#ifdef WIN

/* WINDOWS VERSION */
int getTime(timeStruct * pTS)
{
	SYSTEMTIME time;
	GetSystemTime(&time);
	pTS->year = time.wYear;
	pTS->mon = time.wMonth;
	pTS->day = time.wDay;
	pTS->hour = time.wHour;
	pTS->min = time.wMinute;
	pTS->sec = time.wSecond;
	pTS->milli = time.wMilliseconds;
	return 0;
}

#else

/* POSIX VERSION */
int getTime(timeStruct * pTS)
{
	time_t seconds;
	long milliseconds;
	struct tm *tm;
	struct timespec spec;
	int stat;

	stat = clock_gettime(CLOCK_REALTIME, &spec);
	if (stat != 0) {
		log_error("clock_gettime failed. (posix) \n");
		return 1;
	}

	milliseconds = round(spec.tv_nsec / 1.0e6);
	seconds = spec.tv_sec;
	tm = gmtime(&seconds);

	pTS->year = tm->tm_year + 1900;
	pTS->mon = tm->tm_mon + 1;
	pTS->day = tm->tm_mday;
	pTS->hour = tm->tm_hour;
	pTS->min = tm->tm_min;
	pTS->sec = tm->tm_sec;
	pTS->milli = milliseconds;
	return 0;
}
#endif
