#include <stdio.h>
#include <stdlib.h>
#include "../spectrometer.h"
#pragma GCC diagnostic ignored "-Wstrict-prototypes"
#include "../seabreeze/include/api/seabreezeapi/SeaBreezeAPI.h"
#pragma GCC diagnostic pop

static long deviceID;
static long featureID;
static int spectrum_length;

static double * wavelengths;
static double * spectra;

/*
 *
 */
int spectrometer_init(void){
	int status = 0;
	int error_code = 0;

	sbapi_initialize();

	/* seabreeze_set_verbose(1); */
	int devices = sbapi_probe_devices();

	int number_of_ids = sbapi_get_number_of_device_ids();
	long ids[number_of_ids];
	printf("number_of_ids %i \n", number_of_ids);

	int found_devices = sbapi_probe_devices();
	if(found_devices == 0){
		printf("sbapi_probe_devices: %i\n", status);
		return 1;
	}
	printf("found_devices %i \n", found_devices);

	int number_of_device_ids = sbapi_get_device_ids(ids, number_of_ids);
	if(number_of_device_ids == 0){
		printf("sbapi_get_device_ids\n");
		return 1;
	}
	printf("device_id[0] %lu of %i \n", ids[0], number_of_ids);
	deviceID = ids[0];


	status = sbapi_open_device(deviceID, &error_code);
	if(status){

		const char* error = sbapi_get_error_string(error_code);

		printf("sbapi_open_device. status: %i, error_code: %i\n translates to %s \n", status, error_code, error);
		return 1;
	}


	int length = 100;
	char buffer[length];
	sbapi_get_device_type(deviceID, &error_code, buffer, length);
	printf("device type: %s\n", buffer);


	int number_of_spectrometer_features = sbapi_get_number_of_spectrometer_features(deviceID, &error_code);
	if(error_code != 0){
		printf("sbapi_get_number_of_spectrometer_features. status: %i, error_code: %i \n", status, error_code);
		return 1;
	}
	printf("number_of_spectrometer_features %i \n", number_of_spectrometer_features);

	long features[number_of_spectrometer_features];
	sbapi_get_spectrometer_features(deviceID, &error_code, features, number_of_spectrometer_features);
	if(error_code != 0){
		printf("sbapi_get_spectrometer_features. error_code: %i \n", error_code);
		return 1;
	}
	featureID = features[0];
	printf("featureID is %lu\n", featureID);

	/*
	 *
	 */
	spectrum_length = sbapi_spectrometer_get_formatted_spectrum_length(deviceID, featureID, &error_code);
	if(error_code != 0){
		printf("sbapi_spectrometer_get_formatted_spectrum_length. error_code: %i \n", error_code);
		return 1;
	}
	printf("spectrum_length is %i\n", spectrum_length);



	wavelengths = (double *)malloc(spectrum_length*sizeof(double));
	spectra = (double *)malloc(spectrum_length*sizeof(double));

	return 0;
}

/*That strongly depends on the circumstances
 *
 */
int spectrometer_get(double * wavelengths, double * spectra, int * length){
	int error_code;
	*length = spectrum_length;
	/*
	 *
	 */
	sbapi_spectrometer_get_formatted_spectrum(deviceID, featureID, &error_code, spectra, *length);
	if(error_code != 0){
		printf("sbapi_spectrometer_get_formatted_spectrum. error_code: %i \n", error_code);
		return 1;
	}

	/*
	 *
	 */
	//~ double wavelengths[length];
	sbapi_spectrometer_get_wavelengths( deviceID, featureID, &error_code, wavelengths, *length);
	if(error_code != 0){
		printf("sbapi_spectrometer_get_wavelengths. error_code: %i \n", error_code);
		return 1;
	}

	return 0;

}

/*
 *
 */
int spectrometer_uninit(void){
	int error_code;
	sbapi_close_device(deviceID, &error_code);

	free(spectra);
	free(wavelengths);

	return error_code;
}
