#include <stdio.h>
#include "spectrometer.h"
#include "spectroscopy.h"

int main(int argc, char *argv[])
{
	sSpectrometerStruct spectro;
	printf("init \n");
	spectroscopy_init(&spectro);


	printf("Please cover the spectrometer lens");
	getchar();
	printf("Spectrometer lens covered.\n");
	spectroscopy_calibrate(&spectro);

	printf("Please open the spectrometer lens");
	getchar();
	printf("Spectrometer lens opened.\n");
	spectroscopy_measure(&spectro);

	printf("done!\n");
	return 0;
}
