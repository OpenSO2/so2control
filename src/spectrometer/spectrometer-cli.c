#include <stdio.h>
#include "spectrometer.h"
#include "spectroscopy.h"

int noOfMeasurementsLeft = 1000;
char * filename;

static void callback(sSpectrometerStruct * config);

int main(int argc, char *argv[])
{
	sSpectrometerStruct spectro;
	sConfigStruct config;
	int status = 0;
	filename = argv[1];

	if (argc != 4) {
		printf("usage: spectrometer-cli <outputfile> <number of scans> <exposure time in ms>\n");
		return 1;
	}
	noOfMeasurementsLeft = strtol(argv[2], NULL, 10) + 1;
	spectro.integration_time_micros = strtol(argv[3], NULL, 10) * 1000;

	/* init */
	status = spectrometer_init(&spectro);
	if(status){
		printf("init spectrometer failed\n");
		return 1;
	}

	/* start callback loop */
	callback(&spectro);

	while(noOfMeasurementsLeft){
		sleepMs(100);
	}

	/* uninit */
	status = spectrometer_uninit(&config);
	if(status){
		printf("uninit failed");
	}

	return 0;
}

static void callback(sSpectrometerStruct * spectro)
{
	int i;
	FILE * pFile;

	printf("left %i \n", noOfMeasurementsLeft);

	noOfMeasurementsLeft--;
	if(noOfMeasurementsLeft) {
		spectrometer_trigger(spectro, callback);
	} else {
		printf("spectrum is %i long\n", spectro->spectrum_length);
		pFile = fopen(filename, "wt");
		if (pFile){
			printf("write to %s\n", filename);
			for(i = 0; i < spectro->spectrum_length; i++){
				fprintf(pFile, "%f %f \n", spectro->wavelengths[i], spectro->lastSpectrum[i]);
			}
		}
		else{
			printf("Something wrong writing to File.\n");
		}
	}
	printf("done callback\n");
}
