/**
 * This file manages spectroscopy
 *
 *
 */
#include "spectroscopy.h"
#include "spectrometer.h"
#include "log.h"

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

/*
 * calculate the relative exposure of the spectrum in the desired region of interest.
 * for SO2 measurements, the roi is between 200 and 300nm.
 * A good exposure is at about 80%.
 */
int spectroscopy_calc_exposure(sSpectrometerStruct * spectro, double * relative_exposure);
int spectroscopy_calc_exposure(sSpectrometerStruct * spectro, double * relative_exposure)
{
	int roi_lower = 200; // FIXME: move to conf
	int roi_upper = 300; // FIXME: move to conf
	double max = 1000; // FIXME: get from spectro
	int i = 0;
	int l = spectro->spectrum_length;
	double * wavelengths = spectro->wavelengths;
	double summ;
	while( l-- ){
		if(wavelengths[l] > roi_lower && wavelengths[l] < roi_upper){
			summ += spectro->lastSpectrum[l];
			i++;
		}
	}
	*relative_exposure = summ/i/max; // should be something between 0..1
	return 0;
}

// we need to calculate to optimal integration time in the relevant spectrum region
// a good value would be about 80% of the max value
int spectroscopy_find_exposure_time(sSpectrometerStruct * spectro);
int spectroscopy_find_exposure_time(sSpectrometerStruct * spectro)
{
	//start with some random value
	int integration_time_micros = 1E9;
	int correct_exposure_time;

	// get a spectrum
	// FIXME

	// calculate the exposure in the roi
	double relative_exposure;
	spectroscopy_calc_exposure(spectro, &relative_exposure);

	// calculate optimal exposuretime from value
	// exposure_time / relative_exposure = max_exposure_time / 1 = correct_exposure_time / .8
	correct_exposure_time =  .8 * integration_time_micros / relative_exposure;

	return correct_exposure_time;
}

int spectroscopy_calibrate(sSpectrometerStruct * spectro)
{
	int i;
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
	int i;
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

int spectroscopy_calc_noise(sSpectrometerStruct * spectro)
{
	double photon_noise, diff;
	double spectrum1[spectro->spectrum_length];
	int i, l;

	l = spectro->spectrum_length;

	// take two measurements in quick succession
	spectroscopy_meanAndSubstract(1, 1 * 1000 * 1000, spectro);

	for(i=0; i<l; i++)
		spectrum1[i] = spectro->lastSpectrum[i];

	spectroscopy_meanAndSubstract(1, 1 * 1000 * 1000, spectro);

	// calc average difference
	diff = 0;
	for(i=0; i<l; i++)
		diff += abs( spectrum1[i] - spectro->lastSpectrum[i] );

	// sigma_I = sigma_D / √2
	photon_noise = diff / M_SQRT2;
	log_debug("photon noise is %f", photon_noise);

	return photon_noise;
}

int spectroscopy_meanAndSubstract(int number_of_spectra, int integration_time_micros, sSpectrometerStruct * spectro)
{
	int i;
	done = 1;
	spectro->integration_time_micros = integration_time_micros;
	noOfMeasurements = number_of_spectra;
	noOfMeasurementsLeft = noOfMeasurements;

	for(i=0; i < spectro->spectrum_length; i++){
		spectrum[i] = 0;
	}

	/* start callback loop */
	callback(spectro);

	while(done){
		sleepMs(100);
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
	return 0;
}

int spectroscopy_uninit(sSpectrometerStruct * spectro)
{
	free(spectrum);
	return 0;
}
