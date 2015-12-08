#include <stdio.h>
#include "spectrometer.h"
#include <stdlib.h>
#include <time.h>


int noOfMeasurementsLeft = 1000;
sConfigStruct config;
char * filename;

static void callback(sConfigStruct * config);

int main(int argc, char *argv[])
{
	int status = 0;
	unsigned long exposuretime;
	int noofscans;
	int length = 0, i = 0;
    time_t ltime; /* calendar time */
	filename = argv[1];

	if (argc != 4) {
		printf("usage: spectrometer-cli <outputfile> <number of scans> <exposure time in ms>\n");
		return 1;
	}
	noofscans = strtol(argv[2], NULL, 10);

	config.integration_time_micros = strtol(argv[3], NULL, 10) * 1000; //
	noOfMeasurementsLeft = noofscans;

	/* init */
	status = spectrometer_init(&config);
	if(status){
		printf("init spectrometer failed\n");
		return 1;
	}

	ltime=time(NULL); /* get current cal time */
	printf("1: %s",asctime( localtime(&ltime) ) );

	spectrometer_trigger(&config, callback);

	while(noOfMeasurementsLeft){
		printf("1: sleep...\n");
		sleep(1);
	}

	printf("2: %s", asctime( localtime(&ltime) ) );

	return 0;
}



static void callback(sConfigStruct * config)
{
printf("callback called \n");
	int status;
	int i;
	FILE * pFile;

	printf("left %i \n", noOfMeasurementsLeft);

	noOfMeasurementsLeft--;
	if(noOfMeasurementsLeft) {
		spectrometer_trigger(config, callback);
	} else {
		printf("spectrum is %i long\n", config->spectrum_length);
		pFile = fopen(filename, "wt");
		if (pFile){
			printf("write to %s\n", filename);
			for(i=0; i < config->spectrum_length; i++){
				fprintf(pFile, "%f %f \n", config->lastSpectrum[i], config->wavelengths[i]);
			}
		}
		else{
			printf("Something wrong writing to File.\n");
		}

		/* uninit */
		status = spectrometer_uninit(config);
		if(status){
			printf("uninit failed");
		}
	}
	printf("done callback\n");
}
