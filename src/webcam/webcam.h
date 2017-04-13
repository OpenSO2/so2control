#ifndef _WEBCAM_H_
#define _WEBCAM_H_
#include "configurations.h"
#include "timehelpers.h"

#ifdef __cplusplus
extern "C" {
#endif

int webcam_init(sConfigStruct *, sWebCamStruct *);
int webcam_get(sWebCamStruct *);
int webcam_uninit(sConfigStruct *, sWebCamStruct *);

#ifdef __cplusplus
}
#endif

#endif /* _WEBCAM_H_ */
