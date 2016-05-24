#include <stdio.h>
#include "spectrometer.h"
#include "spectroscopy.h"
#include "spectrometer-shutter.h"
#include "configurations.h"
#include "log.h"


int mkfilename(char * prefix, char * filename);
int save(char * filename, int length, double * wvl, double * spectro);

int main(void)
{
	int i, state;
	sSpectrometerStruct spectro;
	double noise, exposure, exposure_opt;
	char * filename;
	sConfigStruct config;

	/* init the config struct with empty values */
	config_init_sConfigStruct(&config);

	/*
	 * load values from config values for properties that were not set
	 * on the command line
	 */
	state = config_load_configfile(&config);
	if (state != 0) {
		log_error("loading configuration failed");
		return 1;
	}

	/*
	 * load default values for all properties that were not set on the
	 * cli or in the config file
	 */
	config_load_default(&config);

	/* initiate the logfile and start logging */
	state = log_init(&config);
	if (state != 0) {
		printf("creating a logfile failed. Program is aborting...\n");
		return 1;
	}

	spectroscopy_init(&spectro);
	spectrometer_shutter_init(&config);

	for(i=0; ;i++){
		if(i%10 == 0){
			spectrometer_shutter_close();
			spectroscopy_calibrate(&spectro);
			save("dark_current.dat", spectro.spectrum_length, spectro.wavelengths, spectro.dark_current);
			save("electronic-offset.dat", spectro.spectrum_length, spectro.wavelengths, spectro.electronic_offset);
			spectrometer_shutter_open();
		}

		mkfilename("measurement", filename);
		spectroscopy_measure(&spectro);
		noise = spectroscopy_calc_noise(&spectro);
		exposure = spectroscopy_calc_exposure(&spectro);
		exposure_opt = spectroscopy_find_exposure_time(&spectro);

		printf("exposure was %f, an optimal exposure time would be %f. Noise was %f", exposure, exposure_opt, noise);

		save(filename, spectro.spectrum_length, spectro.wavelengths, spectro.lastSpectrum);
	}

	spectrometer_shutter_uninit(&config);
	spectroscopy_uninit(&spectro);

	return 0;
}


int mkfilename(char * prefix, char * filename)
{
	double timestamp = getTimeStamp();
	sprintf(filename, "%s-%f.dat", prefix, timestamp);
	return 0;
}

int save(char * filename, int length, double * wvl, double * spectro)
{
	int i;
	FILE * f = fopen(filename, "wt");
	if (f){
		for(i = 0; i < length; i++){
			fprintf(f, "%f %f \n", wvl[i], spectro[i]);
		}
	}
	fclose(f);

	return 0;
}
