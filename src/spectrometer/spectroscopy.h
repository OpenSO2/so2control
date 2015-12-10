#ifndef _spectroscopyh_
#define _spectroscopyh_
#include "configurations.h"
int spectroscopy_init(sSpectrometerStruct * spectro);
int spectroscopy_calibrate(sSpectrometerStruct * spectro);
int spectroscopy_meanAndSubstract(int number_of_spectra, int integration_time_micros, sSpectrometerStruct * spectro);
int spectroscopy_run(sSpectrometerStruct * spectro);
int spectroscopy_uninit(sSpectrometerStruct * spectro);
#endif
