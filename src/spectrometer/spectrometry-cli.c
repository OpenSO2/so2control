#include <stdio.h>
#include "spectrometer.h"
#include "spectroscopy.h"

int main(int argc, char *argv[])
{
	FILE * pFile;
	int i;
	sSpectrometerStruct spectro;
	spectroscopy_init(&spectro);

	printf("\n➔ Please cover the spectrometer lens");
	getchar();
	printf("✓ Spectrometer lens covered.\n\n");
	spectroscopy_calibrate(&spectro);

	pFile = fopen("dark-current.dat", "wt");
	if (pFile){
		//~ printf("write to %s\n", "electronic-offset.dat");
		for(i = 0; i < spectro.spectrum_length; i++){
			fprintf(pFile, "%f %f \n", spectro.wavelengths[i], spectro.dark_current[i]);
		}
	}
	pFile = fopen("electronic-offset.dat", "wt");
	if (pFile){
		for(i = 0; i < spectro.spectrum_length; i++){
			fprintf(pFile, "%f %f \n", spectro.wavelengths[i], spectro.electronic_offset[i]);
		}
	}

	printf("\n➔ Please open the spectrometer lens");
	getchar();
	printf("✓ Spectrometer lens opened.\n\n");
	spectroscopy_measure(&spectro);

	return 0;
}
