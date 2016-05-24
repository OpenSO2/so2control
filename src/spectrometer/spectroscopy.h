#ifndef _spectroscopyh_
#define _spectroscopyh_
#include "configurations.h"
int spectroscopy_init(sSpectrometerStruct * spectro);
int spectroscopy_calibrate(sSpectrometerStruct * spectro);
int spectroscopy_meanAndSubstract(int number_of_spectra, int integration_time_micros, sSpectrometerStruct * spectro);
int spectroscopy_uninit(sSpectrometerStruct * spectro);
double spectroscopy_calc_noise(sSpectrometerStruct * spectro);
int spectroscopy_measure(sSpectrometerStruct * spectro);

double spectroscopy_find_exposure_time(sSpectrometerStruct * spectro);
double spectroscopy_calc_exposure(sSpectrometerStruct * spectro);
#endif
