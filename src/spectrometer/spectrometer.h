#if !defined(spectrometerh)
#define spectrometerh 1

#include "configurations.h"

int spectrometer_init(sConfigStruct * config);
int spectrometer_uninit(sConfigStruct * config);
int spectrometer_get(double * wavelengths, double * spectra, int length);
int spectrometer_trigger(sConfigStruct * config, void (*callback) (sConfigStruct * config));

#endif