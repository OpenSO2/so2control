#include <stdio.h>
#include <stdlib.h>
#include "../spectrometer.h"

#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include "configurations.h"
#include "getBufferFromFile.c"

int spectrometer_init(sConfigStruct * config)
{
	config->wavelengths = getBufferFromFile("wavelengths.dat", 0);
	return 0;
}
int spectrometer_get(double * wavelengths, double * spectra, int length)
{
	return 0;
}
int spectrometer_uninit(sConfigStruct * config)
{
	return 0;
}
int spectrometer_trigger(sConfigStruct * config, void (*callback) (sConfigStruct * config))
{
	config->lastSpectrum = getBufferFromFile("spectrum.dat", 0);
	callback(config);
	return 0;
}
