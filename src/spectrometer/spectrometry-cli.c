#include <stdio.h>
#include "spectrometer.h"
#include "spectroscopy.h"

int main(void)
{
	FILE * pFile;
	int i;
	sSpectrometerStruct spectro;
	double noise, exposure, exposure_opt;

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


	noise = spectroscopy_calc_noise(&spectro);
	exposure = spectroscopy_calc_exposure(&spectro);
	exposure_opt = spectroscopy_find_exposure_time(&spectro);

	printf("exposure was %f, an optimal exposure time would be %f. Noise was %f \n", exposure, exposure_opt, noise);

	return 0;
}
