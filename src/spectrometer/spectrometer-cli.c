#include <stdio.h>
#include "spectrometer.h"
#include "spectroscopy.h"

int main(int argc, char *argv[])
{
	int i;
	FILE * pFile;
	sSpectrometerStruct spectro;
	sConfigStruct config;
	int status = 0;

	if (argc != 2 && argc != 3) {
		printf("usage: spectrometer-cli <exposure time in ms> [<outputfile]\n");
		return 1;
	}

	if (argc == 3) {
		pFile = fopen(argv[2], "wt");
	} else {
		pFile = stdout;
	}

	status = spectroscopy_init(&spectro);
	if(status){
		printf("init spectrometer failed\n");
		return 1;
	}

	spectro.integration_time_micros = strtol(argv[1], NULL, 10) * 1000;

	status = spectrometer_get(&spectro);
	if(status){
		printf("could not get spectrum\n");
		return 1;
	}

	if (pFile){
		for(i = 0; i < spectro.spectrum_length; i++){
			fprintf(pFile, "%f %f \n", spectro.wavelengths[i], spectro.lastSpectrum[i]);
		}
	} else{
		printf("Something wrong writing to file.\n");
	}

	/* uninit */
	status = spectrometer_uninit(&config);
	if(status){
		printf("uninit failed");
	}

	return 0;
}
