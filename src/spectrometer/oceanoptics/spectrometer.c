/*
 *
 * live cycle
 *
 * spectro init
 *
 * spectro get
 * spectro get
 * spectro get
 *
 * spectro calib
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "configurations.h"
#include "timehelpers.h"
#include "../spectrometer.h"
#pragma GCC diagnostic ignored "-Wstrict-prototypes"
#include "api/seabreezeapi/SeaBreezeAPI.h"
#pragma GCC diagnostic pop

#ifdef WIN
#include <windows.h>
#else
#include <unistd.h>
#endif

static long deviceID;
static long featureID;
/*
 *
 */
int spectrometer_init(sSpectrometerStruct * spectro){
	int error_code = 0;
	int number_of_ids;
	int found_devices;
	int number_of_device_ids;
	int number_of_spectrometer_features;
	int length = 0;
	int status = 0;
	char buffer[length];

	sbapi_initialize();

	found_devices = sbapi_probe_devices();
	if(found_devices == 0){
		log_debug("sbapi_probe_devices: %i", status);
		return 1;
	}
	log_debug("Found %i devices", found_devices);

	number_of_ids = sbapi_get_number_of_device_ids();
	long ids[number_of_ids];
	log_debug("Found %i device IDs", number_of_ids);

	number_of_device_ids = sbapi_get_device_ids(ids, number_of_ids);
	if(number_of_device_ids == 0){
		log_error("sbapi_get_device_ids");
		return 1;
	}
	log_debug("ID of first device is %lu of %i device(s)", ids[0], number_of_ids);
	deviceID = ids[0];


	status = sbapi_open_device(deviceID, &error_code);
	if(status){
		const char* error = sbapi_get_error_string(error_code);

		log_debug("sbapi_open_device. status: %i, error_code: %i, translates to %s", status, error_code, error);
		return 1;
	}

	number_of_spectrometer_features = sbapi_get_number_of_spectrometer_features(deviceID, &error_code);
	if(error_code != 0){
		log_debug("sbapi_get_number_of_spectrometer_features. status: %i, error_code: %i", status, error_code);
		return 1;
	}
	log_debug("Spectrometer has %i features", number_of_spectrometer_features);

	long features[number_of_spectrometer_features];
	sbapi_get_spectrometer_features(deviceID, &error_code, features, number_of_spectrometer_features);
	if(error_code != 0){
		log_debug("sbapi_get_spectrometer_features. error_code: %i", error_code);
		return 1;
	}
	featureID = features[0];
	log_debug("ID of first feature is %lu", featureID);

	/*
	 *
	 */
	spectro->spectrum_length = sbapi_spectrometer_get_formatted_spectrum_length(deviceID, featureID, &error_code);
	if(error_code != 0){
		log_debug("sbapi_spectrometer_get_formatted_spectrum_length. error_code: %i", error_code);
		return 1;
	}
	log_debug("Spectrometer spectrum length is %i", spectro->spectrum_length);

	spectro->lastSpectrum = (double *)calloc(spectro->spectrum_length, sizeof(double));
	spectro->wavelengths = (double *)calloc(spectro->spectrum_length, sizeof(double));

	sbapi_spectrometer_get_wavelengths(deviceID, featureID, &error_code, spectro->wavelengths, spectro->wavelengths);
	if(error_code != 0){
		log_debug("sbapi_spectrometer_get_wavelengths. error_code: %i", error_code);
		return 1;
	}

	spectro->max = 4096;

	spectro->timestampBefore = malloc(sizeof(timeStruct));
	spectro->timestampAfter = malloc(sizeof(timeStruct));

	return 0;
}

/*
 *
 */
int spectrometer_get(sSpectrometerStruct * spectro)
{
	int error_code = 0;
	long time;

	log_debug("set integration_time_micros: %i on device %i, feature %i, error_code %i", (int)spectro->integration_time_micros, (int)deviceID, (int)featureID, error_code);
	sbapi_spectrometer_set_integration_time_micros(deviceID, featureID, &error_code, spectro->integration_time_micros);
	if(error_code != 0){
		const char* error = sbapi_get_error_string(error_code);
		log_debug("sbapi_spectrometer_set_integration_time_micros. error_code: %i, %s", error_code, error);
		return 1;
	}

	//FIXME: Explain

	do {
		time = getTimeStamp();
		sbapi_spectrometer_get_formatted_spectrum(deviceID, featureID, &error_code, spectro->lastSpectrum, spectro->spectrum_length);
		if(error_code != 0){
			const char* error = sbapi_get_error_string(error_code);
			log_error("failed to get formatted spectrum");
			log_debug("sbapi_spectrometer_get_formatted_spectrum. error_code: %i, translates to %s", error_code, error);
			return 1;
		}
		log_debug("spectrum took %i ms; was supposed to take %i", (int)(getTimeStamp() - time), (int)(spectro->integration_time_micros/1000));

	} while(getTimeStamp() - time < (spectro->integration_time_micros/1000)*.95);

	return 0;
}

int spectrometer_uninit(sConfigStruct * config){
	int error_code = 0;
	sbapi_close_device(deviceID, &error_code);

	return error_code;
}
