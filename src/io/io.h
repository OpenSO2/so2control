#ifndef _IO_
#define _IO_

#include "log.h"
#include "configurations.h"

#ifdef __cplusplus
extern "C" {
#endif

int io_init(sConfigStruct * config);
int io_write(sParameterStruct * sSO2Parameters, sConfigStruct * config);
int io_writeWebcam(sWebCamStruct * webcam, sConfigStruct * config);
int io_spectrum_save(sSpectrometerStruct * spectro, sConfigStruct * config);
int io_spectrum_save_calib(sSpectrometerStruct * spectro, sConfigStruct * config);
int io_uninit(sConfigStruct * config);

#ifdef __cplusplus
}
#endif

#endif
