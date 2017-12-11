#ifndef _IO_
#define _IO_

#include "log.h"
#include "configurations.h"

#ifdef __cplusplus
extern "C" {
#endif

int io_init(sConfigStruct *);
int io_write(sParameterStruct *, sConfigStruct *);
int io_writeWebcam(sWebCamStruct *, sConfigStruct *);
int io_spectrum_save(sSpectrometerStruct *, sConfigStruct *);
int io_spectrum_save_calib(sSpectrometerStruct *, sConfigStruct *);
int io_uninit(sConfigStruct *);

#ifdef __cplusplus
}
#endif
#endif
