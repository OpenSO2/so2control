#include <stdio.h>
#include "spectrometer.h"
#include "spectroscopy.h"

int main(int argc, char *argv[])
{
	sSpectrometerStruct spectro;
	printf("init \n");
	spectroscopy_init(&spectro);


	FILE * pFile;
	int i;

	printf("Please cover the spectrometer lens");
	getchar();
	printf("Spectrometer lens covered.\n");
	spectroscopy_calibrate(&spectro);


	pFile = fopen("dark_current.dat", "wt");
	if (pFile){
		//~ printf("write to %s\n", "electronic-offset.dat");
		for(i = 0; i < spectro.spectrum_length; i++){
			fprintf(pFile, "%f %f \n", spectro.wavelengths[i], spectro.dark_current[i]);
		}
	}
	pFile = fopen("electronic-offset.dat", "wt");
	if (pFile){
		//~ printf("write to %s\n", "electronic-offset.dat");
		for(i = 0; i < spectro.spectrum_length; i++){
			fprintf(pFile, "%f %f \n", spectro.wavelengths[i], spectro.electronic_offset[i]);
		}
	}

	printf("Please open the spectrometer lens");
	getchar();
	printf("Spectrometer lens opened.\n");
	spectroscopy_measure(&spectro);

	printf("done!\n");
	return 0;
}
