#include "spectrometer-shutter.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"
int spectrometer_shutter_init(sConfigStruct * config)
{
	return 0;
}

int spectrometer_shutter_open(void)
{
	printf("\n➔ Please open the spectrometer lens");
	getchar();
	printf("✓ Spectrometer lens opened.\n\n");
	return 0;
}

int spectrometer_shutter_close(void)
{
	printf("\n➔ Please cover the spectrometer lens");
	getchar();
	printf("✓ Spectrometer lens closed.\n\n");
	return 0;
}

int spectrometer_shutter_uninit(sConfigStruct * config)
{
	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"
