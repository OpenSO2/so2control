#ifndef _WEBCAM_H_
#define _WEBCAM_H_
#include "highgui.h"
#include "configurations.h"

int webcam_init(sWebCamStruct *camStruct);
int webcam_get(sWebCamStruct *camStruct);
int webcam_uninit(sWebCamStruct *camStruct);

#endif /* _WEBCAM_H_ */
