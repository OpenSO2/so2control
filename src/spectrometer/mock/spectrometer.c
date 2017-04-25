#include <stdlib.h>
#include "spectrometer.h"
#include "log.h"
#include "configurations.h"

static double *wavelengths = NULL;
static double *spectrum = NULL;

int spectrometer_init(sSpectrometerStruct * spectro)
{
	int i;
	int number_of_lines = 0;
	char line[512];

	FILE *f = fopen(SPECTROMETER_MOCK_WAVELENGTHS, "r");
	if (!f) {
		log_error("failed to open file");
		return 1;
	}

	while (fgets(line, 512, f) != NULL) {
		number_of_lines++;
	}

	wavelengths = calloc(number_of_lines, sizeof(double));
	spectrum = calloc(number_of_lines, sizeof(double));

	spectro->spectrum_length = number_of_lines;
	spectro->max = 4096;

	/* rewind file */
	fseek(f, 0, SEEK_SET);

	i = 0;
	while (fgets(line, 512, f) != NULL) {
		wavelengths[i++] = atol(line);
	}

	fclose(f);

	f = fopen(SPECTROMETER_MOCK_SPECTRUM, "r");
	if (!f) {
		log_error("failed to open file");
		return 1;
	}

	i = 0;
	while (fgets(line, 512, f) != NULL) {
		spectrum[i++] = atol(line);
	}

	fclose(f);

	spectro->timestampBefore = malloc(sizeof(timeStruct));
	spectro->timestampAfter = malloc(sizeof(timeStruct));

	return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int spectrometer_uninit(sConfigStruct * config)
{
	free(wavelengths);
	free(spectrum);
	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"

#pragma GCC diagnostic ignored "-Wunused-parameter"
int spectrometer_get(sSpectrometerStruct * spectro)
{
	sleep(spectro->integration_time_micros / 1000000);
	spectro->lastSpectrum = spectrum;
	spectro->wavelengths = wavelengths;

	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"
