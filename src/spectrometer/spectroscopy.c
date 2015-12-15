/**
 * This file manages spectroscopy
 *
 *
 */

#include "spectroscopy.h"

static int noOfMeasurementsLeft = 0;
static int noOfMeasurements = 0;
double * spectrum;

int spectroscopy_init(sSpectrometerStruct * spectro)
{
	int status = spectrometer_init(spectro);
	if(status){
		printf("init spectrometer failed\n");
		return 1;
	}

	spectrum = calloc(spectro->spectrum_length, sizeof(double));
	spectro->electronic_offset = calloc(spectro->spectrum_length, sizeof(double));
	spectro->dark_current = calloc(spectro->spectrum_length, sizeof(double));

	return 0;
}

int spectroscopy_calibrate(sSpectrometerStruct * spectro)
{
	int i = 0;
	FILE * pFile;
	int dark_current_integration_time_s = 60;

	printf("… Measuring electronic offset \n");
	// measure electronic offset
	spectroscopy_meanAndSubstract(10000, 3000, spectro);

	// copy last spectrum into electronic_offset buffer
	for (i = 0; i < spectro->spectrum_length; i++) {
		spectro->electronic_offset[i] = spectrum[i];

		spectro->lastSpectrum[i] = 0;
		spectrum[i] = 0.;
	}

	printf("… Measuring dark current \n");
	spectroscopy_meanAndSubstract(1, dark_current_integration_time_s * 1000 * 1000, spectro);

	// copy last spectrum into dark_current buffer
	for (i = 0; i < spectro->spectrum_length; i++) {
		spectro->dark_current[i] = spectro->lastSpectrum[i];

		spectro->lastSpectrum[i] = 0;
		spectrum[i] = 0.;
	}

	return 0;
}

static int done = 1;

static void callback(sSpectrometerStruct * spectro)
{
	int status, i;
	if(noOfMeasurementsLeft == 0) {
		done = 0;
	} else {
		for (i = 0; i < spectro->spectrum_length; i++) {

			//~ printf("applying electronic offset correction: %f\n", spectro->electronic_offset[i]);
			spectro->lastSpectrum[i] -= spectro->electronic_offset[i]; // substract electronic offset, scaled by number of measurements (electronic offset was averaged to 1 measurement)

			//~ printf("applying dark current correction: %f of %f to %f \n", spectro->dark_current[i] * spectro->integration_time_micros/60000000, spectro->dark_current[i], spectro->lastSpectrum[i]);
			spectro->lastSpectrum[i] -= spectro->dark_current[i] * spectro->integration_time_micros/60000000; // substract dark current, scaled by integration time

			spectrum[i] += spectro->lastSpectrum[i] / noOfMeasurements;
		}
		noOfMeasurementsLeft--;

		spectrometer_trigger(spectro, callback);
 	}
}

int spectroscopy_meanAndSubstract(int number_of_spectra, int integration_time_micros, sSpectrometerStruct * spectro)
{
	done = 1;
	int status = 0;
	int i;
	spectro->integration_time_micros = integration_time_micros;
	noOfMeasurements = number_of_spectra;
	noOfMeasurementsLeft = noOfMeasurements;

	for(i=0; i < spectro->spectrum_length; i++){
		spectrum[i] = 0;
	}

	/* start callback loop */
	callback(spectro);

	while(done){
		sleepMs(1000);
	}
	printf("✓ Measurement done\n\n");

	return 0;
}

int spectroscopy_measure(sSpectrometerStruct * spectro)
{
	FILE * pFile;
	int i;

	for (i = 0; i < spectro->spectrum_length; i++) {
		spectrum[i] = 0.;
		spectro->lastSpectrum[i] = 0.;
	}

	// measure dark current
	spectroscopy_meanAndSubstract(100, 300000, spectro);

	pFile = fopen("measurement.dat", "wt");
	if (pFile){
		//~ printf("write to %s\n", "electronic-offset.dat");
		for(i = 0; i < spectro->spectrum_length; i++){
			fprintf(pFile, "%f %f \n", spectro->wavelengths[i], spectrum[i]);
		}
	}

}

int spectroscopy_run(sSpectrometerStruct * spectro)
{

}

int spectroscopy_uninit(sSpectrometerStruct * spectro)
{
	free(spectrum);
}
