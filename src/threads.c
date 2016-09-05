#include<pthread.h>
#include "threads.h"
#include "configurations.h"
#include "webcam.h"
#include "log.h"
#include "io.h"

pthread_t id = 0;

struct webcam_struct{
	sConfigStruct * config;
	sWebCamStruct * webcam;
};

struct webcam_struct *webcam_s;


void *threads_webcam_run(void *args);


int threads_webcam_start(sConfigStruct * config, sWebCamStruct * webcam)
{
	webcam_s = (struct webcam_struct*) calloc(1, sizeof(*webcam_s));

	webcam_s->webcam = webcam;
	webcam_s->config = config;

	pthread_create(&id, NULL, &threads_webcam_run, webcam_s);

	return 0;
}


void * threads_webcam_run(void *args)
{
	int status;
	sWebCamStruct *webcam = ((struct webcam_struct*) args)->webcam;
	sConfigStruct *config = ((struct webcam_struct*) args)->config;

	while( 1 ){
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
}


int threads_webcam_stop(void)
{
	void * res;

	if(!id){
		pthread_cancel(id);
		pthread_join(id, &res);
	}
	return 0;
}
