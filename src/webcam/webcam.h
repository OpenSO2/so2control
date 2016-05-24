#ifndef _WEBCAM_H_
#define _WEBCAM_H_
#include "configurations.h"
#include "timehelpers.h"

int webcam_init(sConfigStruct * config, sWebCamStruct * webcam);
int webcam_get(sWebCamStruct * webcam);
int webcam_uninit(sConfigStruct * config);

#endif /* _WEBCAM_H_ */
