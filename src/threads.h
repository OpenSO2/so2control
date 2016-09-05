#ifndef _THREADS_H_
#define _THREADS_H_

#include<pthread.h>
#include "configurations.h"
#include "webcam.h"

int threads_webcam_start(sConfigStruct * config, sWebCamStruct * webcam);
int threads_webcam_stop(void);

#endif
