#ifndef _spectroscopyh_
#define _spectroscopyh_
#include "configurations.h"
int spectroscopy_init(sConfigStruct *, sSpectrometerStruct *);
int spectroscopy_calibrate(sSpectrometerStruct *);
int spectroscopy_meanAndSubstract(int, int, sSpectrometerStruct *);
int spectroscopy_uninit(sSpectrometerStruct *);
double spectroscopy_calc_noise(sSpectrometerStruct *);
int spectroscopy_measure(sSpectrometerStruct *);

double spectroscopy_find_exposure_time(sSpectrometerStruct *);
double spectroscopy_calc_exposure(sSpectrometerStruct *);
#endif
