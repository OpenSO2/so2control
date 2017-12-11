#include "spectrometer-shutter.h"
#include "log.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
int spectrometer_shutter_init(sConfigStruct * config)
{
	return 0;
}

int spectrometer_shutter_open(void)
{
	log_message("mock spectrometer shutter: open");
	return 0;
}

int spectrometer_shutter_close(void)
{
	log_message("mock spectrometer shutter: close");
	return 0;
}

int spectrometer_shutter_uninit(sConfigStruct * config)
{
	return 0;
}

#pragma GCC diagnostic warning "-Wunused-parameter"
