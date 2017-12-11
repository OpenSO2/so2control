/**
 * This file manages spectroscopy
 *
 *
 */
#include "spectroscopy.h"
#include "spectrometer.h"
#include "log.h"

int spectroscopy_roi_upper;
int spectroscopy_roi_lower;

int spectroscopy_init(sConfigStruct * config, sSpectrometerStruct * spectro)
{
	int status = spectrometer_init(spectro);
	if (status) {
		log_error("init spectroscopy failed");
		return 1;
	}

	spectro->electronic_offset = calloc(spectro->spectrum_length, sizeof(double));
	spectro->dark_current = calloc(spectro->spectrum_length, sizeof(double));

	spectroscopy_roi_upper = config->spectroscopy_roi_upper;
	spectroscopy_roi_lower = config->spectroscopy_roi_lower;

	/* set 1s as initial integration time */
	spectro->integration_time_micros = 1000 * 1000;

	return 0;
}

/*
 * Calculate the absolute exposure of the last spectrum in the desired region of interest.
 * For SO2 measurements, the roi is between 300 and 325nm.
 */
double spectroscopy_calc_exposure(sSpectrometerStruct * spectro)
{
	int l = spectro->spectrum_length;
	double *wavelengths = spectro->wavelengths;
	double highest = 0;
	while (l--) {
		if (wavelengths[l] > spectroscopy_roi_lower && wavelengths[l] < spectroscopy_roi_upper) {
			if (spectro->lastSpectrum[l] > highest)
				highest = spectro->lastSpectrum[l];
		}
	}

	return highest;		// should be something between 0..max
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
	log_message("spectro: find exposure time inttime: %i, max: %f, exposure: %f", spectro->integration_time_micros, spectro->max, spectroscopy_calc_exposure(spectro));
	return .7 * spectro->integration_time_micros * spectro->max / spectroscopy_calc_exposure(spectro);
}

int spectroscopy_calibrate(sSpectrometerStruct * spectro)
{
	int i;
	int dark_current_integration_time_s = 60;

	log_message("Spectroscopy: Measuring electronic offset");
	/* measure electronic offset */
	spectroscopy_mean(10000, 3000, spectro);

	/* copy last spectrum into electronic_offset buffer */
	for (i = 0; i < spectro->spectrum_length; i++) {
		spectro->electronic_offset[i] = spectro->lastSpectrum[i];
		spectro->lastSpectrum[i] = 0;
	}

	log_message("Spectroscopy: Measuring dark current");
	spectroscopy_mean(1, dark_current_integration_time_s * 1000 * 1000, spectro);

	/* copy last spectrum into dark_current buffer */
	for (i = 0; i < spectro->spectrum_length; i++) {
		spectro->dark_current[i] = spectro->lastSpectrum[i];
		spectro->lastSpectrum[i] = 0;
	}

	return 0;
}

double spectroscopy_calc_noise(sSpectrometerStruct * spectro)
{
	double photon_noise, diff;
	double spectrum[spectro->spectrum_length];
	int i, l;

	l = spectro->spectrum_length;

	int old_integration_time = spectro->integration_time_micros;

	// take two measurements in quick succession
	spectroscopy_meanAndSubstract(1, 1 * 1000 * 1000, spectro);

	for (i = 0; i < l; i++)
		spectrum[i] = spectro->lastSpectrum[i];

	spectroscopy_meanAndSubstract(1, 1 * 1000 * 1000, spectro);

	// calc std deviation
	// calc average difference
	diff = 0;
	for (i = 0; i < l; i++)
		diff += abs(spectrum[i] - spectro->lastSpectrum[i]);

	// sigma_I = sigma_D / √2
	photon_noise = diff / M_SQRT2;
	log_debug("Spectroscopy: photon noise is %f", photon_noise);

	spectro->integration_time_micros = old_integration_time;

	return photon_noise;
}

/*
 * After this operation the result is saved in spectro->lastSpectrum
 * call io_spectrum_save(spectro, config) to save to disk
 */
int spectroscopy_meanAndSubstract(int noOfMeasurements, int integration_time_micros, sSpectrometerStruct * spectro)
{
	int i, j;
	spectro->integration_time_micros = integration_time_micros;
	spectro->scans = noOfMeasurements;

	double *spectrum = (double *)calloc(spectro->spectrum_length, sizeof(double));
	for (i = 0; i < spectro->spectrum_length; i++) {
		spectrum[i] = 0;
	}

	for (j = 0; j < noOfMeasurements; j++) {
		spectrometer_get(spectro);

		for (i = 0; i < spectro->spectrum_length; i++) {
			spectro->lastSpectrum[i] -= spectro->electronic_offset[i];	// substract electronic offset, scaled by number of measurements (electronic offset was averaged to 1 measurement)
			spectro->lastSpectrum[i] -= spectro->dark_current[i] * spectro->integration_time_micros / 60000000;	// substract dark current, scaled by integration time
			spectrum[i] += spectro->lastSpectrum[i] / noOfMeasurements;
		}
	}

	for (i = 0; i < spectro->spectrum_length; i++) {
		spectro->lastSpectrum[i] = spectrum[i];
	}

	free(spectrum);

	return 0;
}

int spectroscopy_mean(int number_of_spectra, int integration_time_micros, sSpectrometerStruct * spectro)
{
	double *spectrum = (double *)calloc(spectro->spectrum_length, sizeof(double));
	int noOfMeasurements = number_of_spectra;
	int i;
	for (i = 0; i < spectro->spectrum_length; i++) {
		spectrum[i] = 0;
	}

	spectro->integration_time_micros = integration_time_micros;

	while (number_of_spectra--) {
		spectrometer_get(spectro);

		for (i = 0; i < spectro->spectrum_length; i++) {
			spectrum[i] += spectro->lastSpectrum[i];
		}
	}

	for (i = 0; i < spectro->spectrum_length; i++) {
		spectro->lastSpectrum[i] = spectrum[i] / noOfMeasurements;
	}

	spectro->scans = noOfMeasurements;

	free(spectrum);

	return 0;
}

/*
 * take a measurement and save in spectro.lastSpectrum
 *
 */
int spectroscopy_measure(sSpectrometerStruct * spectro)
{
	spectroscopy_meanAndSubstract(10, spectro->integration_time_micros, spectro);
	log_message("Spectroscopy: measurement done");
	return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int spectroscopy_uninit(sSpectrometerStruct * spectro)
{
	return 0;
}

#pragma GCC diagnostic warning "-Wunused-parameter"
