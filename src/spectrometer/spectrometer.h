#if !defined(spectrometerh)
#define spectrometerh 1

#include "configurations.h"

int spectrometer_init(sSpectrometerStruct * spectro);
int spectrometer_uninit(sConfigStruct * config);
int spectrometer_trigger(sSpectrometerStruct * config, void (*callback) (sSpectrometerStruct * config));

#endif
