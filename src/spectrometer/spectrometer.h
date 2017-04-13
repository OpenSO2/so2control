#if !defined(spectrometerh)
#define spectrometerh 1

#include "configurations.h"

int spectrometer_init(sSpectrometerStruct *);
int spectrometer_get(sSpectrometerStruct *);
int spectrometer_uninit(sConfigStruct *);

#endif
