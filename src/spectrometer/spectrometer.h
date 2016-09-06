#if !defined(spectrometerh)
#define spectrometerh 1

#include "configurations.h"

int spectrometer_init(sSpectrometerStruct * spectro);
int spectrometer_get(sSpectrometerStruct * config);
int spectrometer_uninit(sConfigStruct * config);

#endif
