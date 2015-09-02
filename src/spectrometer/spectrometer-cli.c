#include <stdio.h>
#include "spectrometer.h"

int main(void){
	int status = 0;
	double * wavelengths;
	double * spectra;
	int length, i;
	FILE * pFile;

	/* init */
	spectrometer_init();
	if(status){
		printf("getting spectra failed");
	}

	/* get and save/plot */
	status = spectrometer_get(wavelengths, spectra, &length);
	if(status){
		printf("getting spectra failed");
	}

	pFile = fopen("spectrum.txt", "wt");
	if (pFile){
		for(i=0; i < length; i++){
			fprintf(pFile, "%f %f", wavelengths[i], spectra[i]);
		}
	}
	else{
		printf("Something wrong writing to File.\n");
	}

	/* uninit */
	status = spectrometer_uninit();
	if(status){
		printf("uninit failed");
	}

	return 0;
}
