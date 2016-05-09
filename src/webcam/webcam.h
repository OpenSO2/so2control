#ifndef _WEBCAM_H_
#define _WEBCAM_H_
#include "opencv/highgui.h"
#include "configurations.h"

int webcam_init(sConfigStruct * config);
int webcam_get(sWebCamStruct * camStruct);
int webcam_uninit(sConfigStruct * config);

#endif /* _WEBCAM_H_ */
