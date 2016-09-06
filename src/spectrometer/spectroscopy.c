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
		printf("init spectroscopy failed\n");
		return 1;
	}

	spectrum = calloc(spectro->spectrum_length, sizeof(double));
	spectro->electronic_offset = calloc(spectro->spectrum_length, sizeof(double));
	spectro->dark_current = calloc(spectro->spectrum_length, sizeof(double));

	/* set 1s as initial integration time */
	spectro->integration_time_micros = 1000 * 1000;

	return 0;
}

/*
 * Calculate the absolute exposure of the spectrum in the desired region of interest.
 * For SO2 measurements, the roi is between 200 and 300nm.
 */
double spectroscopy_calc_exposure(sSpectrometerStruct * spectro)
{
	int roi_lower = 250; // FIXME: move to conf
	int roi_upper = 350; // FIXME: move to conf
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
	return summ/i; // should be something between 0..max
}

/*
 * We need to calculate to optimal integration time in the relevant spectrum region
 * The saturation T of a detector pixel P is defined as:
 *           C(P)
 * T(P) = ----------
 *         N · Cmax
 * C = counts in pixel
 * N = number of scans = 1
 * C_max = 4096 for the USB2000+
 *
 * above 80% the sensitivity drops of, so a good saturation is at about .7, thus:
 * T_opt = C_opt/Cmax = .7
 *
 * if t is the exposure time, since T ~ t
 *
 * T_opt     t_opt
 * -----  =  -----
 * T_arb     t_arb
 *
 * Thus:
 *           .7 · t_arb
 * t_opt =   ----------   = .7 · t_arb · Cmax / C_arb
 *              T_arb
 *
*/
double spectroscopy_find_exposure_time(sSpectrometerStruct * spectro)
{
	return .7 * spectro->integration_time_micros  * spectro->max /  spectroscopy_calc_exposure(spectro);
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

double spectroscopy_calc_noise(sSpectrometerStruct * spectro)
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

	// calc std deviation
	// calc average difference
	diff = 0;
	for(i=0; i<l; i++)
		diff += abs( spectrum1[i] - spectro->lastSpectrum[i] );

	// sigma_I = sigma_D / √2
	photon_noise = diff / M_SQRT2;
	log_debug("photon noise is %f", photon_noise);

	return photon_noise;
}

// FIXME: WHERE IS SPECTRUM SAVED?

int spectroscopy_meanAndSubstract(int number_of_spectra, int integration_time_micros, sSpectrometerStruct * spectro)
{
	int i, j;
	spectro->integration_time_micros = integration_time_micros;
	noOfMeasurements = number_of_spectra;
	noOfMeasurementsLeft = noOfMeasurements;

	for( j = 0; j < noOfMeasurements; j++){
		spectrometer_get(spectro);

		for (i = 0; i < spectro->spectrum_length; i++) {
			//~ printf("applying electronic offset correction: %f\n", spectro->electronic_offset[i]);
			spectro->lastSpectrum[i] -= spectro->electronic_offset[i]; // substract electronic offset, scaled by number of measurements (electronic offset was averaged to 1 measurement)

			//~ printf("applying dark current correction: %f of %f to %f \n", spectro->dark_current[i] * spectro->integration_time_micros/60000000, spectro->dark_current[i], spectro->lastSpectrum[i]);
			spectro->lastSpectrum[i] -= spectro->dark_current[i] * spectro->integration_time_micros/60000000; // substract dark current, scaled by integration time

			spectrum[i] += spectro->lastSpectrum[i] / noOfMeasurements;
		}
 	}

	printf("✓ Measurement done\n\n");

	return 0;
}

/*
 * take a measurement and save in spectro.lastSpectrum
 *
 */
int spectroscopy_measure(sSpectrometerStruct * spectro)
{
	spectroscopy_meanAndSubstract(1, spectro->integration_time_micros, spectro);
	return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int spectroscopy_uninit(sSpectrometerStruct * spectro)
{
	free(spectrum);
	return 0;
}
#pragma GCC diagnostic warning "-Wunused-parameter"
