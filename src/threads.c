#include<pthread.h>
#include "threads.h"
#include "configurations.h"
#include "webcam.h"
#include "log.h"
#include "io.h"
#include "spectrometer.h"
#include "spectroscopy.h"
#include "spectrometer-shutter.h"

pthread_t webcam_threadid = 0;

struct webcam_struct{
	sConfigStruct * config;
	sWebCamStruct * webcam;
};

struct webcam_struct * webcam_s = NULL;


void * threads_webcam_run(void * args);

int webcam_running = 1;
pthread_mutex_t webcamMutex;

int getWebcamRunning(void);
int getWebcamRunning(void) {
	int ret = 0;
	pthread_mutex_lock(&webcamMutex);
	ret = webcam_running;
	pthread_mutex_unlock(&webcamMutex);
	return ret;
}

void setWebcamRunning(int);
void setWebcamRunning(int val) {
	pthread_mutex_lock(&webcamMutex);
	webcam_running = val;
	pthread_mutex_unlock(&webcamMutex);
}


int threads_webcam_start(sConfigStruct * config, sWebCamStruct * webcam)
{
	webcam_s = (struct webcam_struct*) calloc(1, sizeof(*webcam_s));

	webcam_s->webcam = webcam;
	webcam_s->config = config;
	setWebcamRunning(1);
	pthread_create(&webcam_threadid, NULL, &threads_webcam_run, webcam_s);

	return 0;
}

void * threads_webcam_run(void * args)
{
	int status;
	sWebCamStruct * webcam = ((struct webcam_struct*) args)->webcam;
	sConfigStruct * config = ((struct webcam_struct*) args)->config;
	int oldtype;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);

	while( getWebcamRunning() ){
		getTime(webcam->timestampBefore);
		status = webcam_get(webcam);
		getTime(webcam->timestampAfter);

		/* save webcam image */
		status = io_writeWebcam(webcam, config);
		if (status != 0) {
			log_error("failed to write webcam image");
		}
		sleep(3);
	}

	#ifdef WIN
	return 0;
	#else
	pthread_exit((void *) 0);
	#endif
}

int threads_webcam_stop(void)
{
	void * res;

	if(webcam_s != NULL){
		free(webcam_s);
	}

	if(webcam_threadid){
		setWebcamRunning(0);
		pthread_join(webcam_threadid, &res);
	}

	return 0;
}


/*====================================================================*/
/* SPECTROMETER THREAD */

pthread_t spectroscopy_threadid = 0;

int spectroscopy_running = 1;
pthread_mutex_t spectroscopyMutex;

int getSpectroscopyRunning(void);
int getSpectroscopyRunning(void) {
	int ret = 0;
	pthread_mutex_lock(&spectroscopyMutex);
	ret = spectroscopy_running;
	pthread_mutex_unlock(&spectroscopyMutex);
	return ret;
}

void setSpectroscopyRunning(int);
void setSpectroscopyRunning(int val) {
	pthread_mutex_lock(&spectroscopyMutex);
	spectroscopy_running = val;
	pthread_mutex_unlock(&spectroscopyMutex);
}


struct spectroscopy_struct{
	sSpectrometerStruct * spectro;
	sConfigStruct * config;
};
struct spectroscopy_struct * spectroscopy_s = NULL;


void * threads_spectroscopy_run(void * args);
void * threads_spectroscopy_run(void * args)
{
	int status;
	sSpectrometerStruct * spectro = ((struct spectroscopy_struct*) args)->spectro;
	sConfigStruct * config = ((struct spectroscopy_struct*) args)->config;
	int oldtype;
	double noise, exposure_opt;
	int i;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);

	/* set integration time to 1s initially for the calibration */
	spectro->integration_time_micros = 1000 * 1000;

	for (i = 0; getSpectroscopyRunning(); i++) {

		if (i % config->spectrometer_calibrate_intervall == 0){
			getTime(spectro->timestampBefore);
			getTime(spectro->timestampAfter);
			spectrometer_shutter_close();
			spectroscopy_calibrate(spectro);
			spectrometer_shutter_open();
			io_spectrum_save_calib(spectro, config);

			status = spectrometer_get(spectro);
			// fixme handle return

			// calculate optimal exposure time
			exposure_opt = spectroscopy_find_exposure_time(spectro);
			log_error("optimal exposure would be %f, but was %i", exposure_opt, spectro->integration_time_micros);
			spectro->integration_time_micros = exposure_opt;

			// calculate number of averages spectra to control noise to a reasonable level
			noise = spectroscopy_calc_noise(spectro);
			// TODO calculate number of mean'd spectra

			log_error("noise was %f", noise);
		} else {
			log_message("open shutter and take another spectrum");
			spectrometer_shutter_open();

			getTime(spectro->timestampBefore);
			status = spectroscopy_measure(spectro);
			getTime(spectro->timestampAfter);

			/* save spectrum */
			status = io_spectrum_save(spectro, config);
			if (status != 0) {
				log_error("failed to write spectrum");
			}
		}
	}

	#ifdef WIN
	return 0;
	#else
	pthread_exit((void *) 0);
	#endif
}

int threads_spectroscopy_start(sConfigStruct * config, sSpectrometerStruct * spectro)
{
	spectroscopy_s = (struct spectroscopy_struct*) calloc(1, sizeof(*spectroscopy_s));

	spectroscopy_s->spectro = spectro;
	spectroscopy_s->config = config;
	setSpectroscopyRunning(1);

	pthread_create(&spectroscopy_threadid, NULL, &threads_spectroscopy_run, spectroscopy_s);

	return 0;
}

int threads_spectroscopy_stop(void)
{
	void * res;

	if(spectroscopy_s != NULL){
		free(spectroscopy_s);
	}

	if(spectroscopy_threadid){
		setSpectroscopyRunning(0);
		pthread_join(spectroscopy_threadid, &res);
	}

	return 0;
}
