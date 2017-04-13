#ifndef _THREADS_H_
#define _THREADS_H_

#include<pthread.h>
#include "configurations.h"
#include "webcam.h"

int threads_webcam_start(sConfigStruct *, sWebCamStruct *);
int threads_webcam_stop(void);

int threads_spectroscopy_start(sConfigStruct *, sSpectrometerStruct *);
int threads_spectroscopy_stop(void);

#endif
