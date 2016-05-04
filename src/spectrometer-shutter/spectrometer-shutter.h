#ifndef _spectrometer_shutter_
#define _spectrometer_shutter_

#include "configurations.h"

int spectrometer_shutter_init(sConfigStruct * config);
int spectrometer_shutter_open(void);
int spectrometer_shutter_close(void);
int spectrometer_shutter_uninit(sConfigStruct * config);
#endif
