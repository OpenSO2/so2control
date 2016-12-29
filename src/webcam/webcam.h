#ifndef _WEBCAM_H_
#define _WEBCAM_H_
#include "configurations.h"
#include "timehelpers.h"

#ifdef __cplusplus
extern "C" {
#endif

int webcam_init(sConfigStruct * config, sWebCamStruct * webcam);
int webcam_get(sWebCamStruct * webcam);
int webcam_uninit(sConfigStruct * config, sWebCamStruct * webcam);

#ifdef __cplusplus
}
#endif

#endif /* _WEBCAM_H_ */
