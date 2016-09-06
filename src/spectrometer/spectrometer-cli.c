#include <stdio.h>
#include "spectrometer.h"
#include "spectroscopy.h"

int main(int argc, char *argv[])
{
	int noOfMeasurementsLeft = 1;
	int integration_time_micros = 1;
	char * filename;
	int i;
	FILE * pFile;
	sSpectrometerStruct spectro;
	sConfigStruct config;
	int status = 0;
	filename = argv[1];

	if (argc != 4) {
		printf("usage: spectrometer-cli <outputfile> <number of scans> <exposure time in ms>\n");
		return 1;
	}
	noOfMeasurementsLeft = strtol(argv[2], NULL, 10) + 1;
	integration_time_micros = strtol(argv[3], NULL, 10) * 1000;

	/* init */
	status = spectrometer_init(&spectro);
	if(status){
		printf("init spectrometer failed\n");
		return 1;
	}

	spectroscopy_meanAndSubstract(noOfMeasurementsLeft, integration_time_micros, &spectro);

	printf("spectrum is %i long\n", spectro.spectrum_length);
	pFile = fopen(filename, "wt");
	if (pFile){
		printf("write to %s\n", filename);
		for(i = 0; i < spectro.spectrum_length; i++){
			fprintf(pFile, "%f %f \n", spectro.wavelengths[i], spectro.lastSpectrum[i]);
		}
	} else{
		printf("Something wrong writing to File.\n");
	}

	/* uninit */
	status = spectrometer_uninit(&config);
	if(status){
		printf("uninit failed");
	}

	return 0;
}
