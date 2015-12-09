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
#include "libseabreeze/SeaBreeze/include/api/seabreezeapi/SeaBreezeAPI.h"
#pragma GCC diagnostic pop

#ifdef WIN
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

static long deviceID;
static long featureID;
static int spectrum_length;
struct data_struct{
	void (*callback)(sConfigStruct * config);
	sConfigStruct * config;
};

/*
 *
 */
int spectrometer_init(sConfigStruct * config){
	int error_code = 0;
	int number_of_ids;
	int number_of_device_ids;
	int number_of_spectrometer_features;
	int length = 100;
	int status = 0;
	char buffer[length];

	sbapi_initialize();

	number_of_ids = sbapi_get_number_of_device_ids();
	long ids[number_of_ids];
	printf("number_of_ids %i \n", number_of_ids);


	number_of_device_ids = sbapi_get_device_ids(ids, number_of_ids);
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

	sbapi_get_device_type(deviceID, &error_code, buffer, length);
	printf("device type: %s\n", buffer);


	number_of_spectrometer_features = sbapi_get_number_of_spectrometer_features(deviceID, &error_code);
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
	config->spectrum_length = spectrum_length;
	printf("spectrum_length is %i\n", spectrum_length);

	config->lastSpectrum = (double *)malloc(spectrum_length * sizeof(double));
	config->wavelengths = (double *)malloc(spectrum_length * sizeof(double));

	sbapi_spectrometer_get_wavelengths(deviceID, featureID, &error_code, config->wavelengths, length);
	if(error_code != 0){
		printf("sbapi_spectrometer_get_wavelengths. error_code: %i \n", error_code);
		return 1;
	}

	return 0;
}


#ifdef WIN
DWORD WINAPI timeout(void * args)
#else
static void * timeout(void * args);
static void * timeout(void * args)
#endif
{
	void (*callback)(sConfigStruct * config) = ((struct data_struct*) args)->callback;
	sConfigStruct * config = ((struct data_struct*) args)->config;

	int error_code = 0;
	long time;

	do {
		time = getTimeStamp();
		sbapi_spectrometer_get_formatted_spectrum(deviceID, featureID, &error_code, config->lastSpectrum, spectrum_length);
		if(error_code != 0){
			printf("sbapi_spectrometer_get_formatted_spectrum. error_code: %i \n", error_code);
		}
		printf("spectrum took %i ms; was supposed to take %i \n", (int)(getTimeStamp() - time), (int)(config->integration_time_micros/1000));

	} while(getTimeStamp() - time < (config->integration_time_micros/1000)*.95);

	callback(config);

	#ifdef WIN
	return 0;
	#else
	pthread_exit((void *) 0);
	#endif
}

/*That strongly depends on the circumstances
 *
 */
int spectrometer_trigger(sConfigStruct * config, void (*callback) (sConfigStruct * config))
{
	#ifdef WIN
	HANDLE thread;
	#else
	pthread_t thread_id;
	#endif

	int error_code = 0;
	struct data_struct * g_data_struct = (struct data_struct*) calloc(1, sizeof(*g_data_struct));
	g_data_struct->callback = callback;
	g_data_struct->config = config;

	/*
	 *
	 */
	printf("set integration_time_micros: %i on device %i, feature %i, error_code %i \n", (int)config->integration_time_micros, (int)deviceID, (int)featureID, error_code);
	sbapi_spectrometer_set_integration_time_micros(deviceID, featureID, &error_code, config->integration_time_micros);
	if(error_code != 0){
		const char* error = sbapi_get_error_string(error_code);
		printf("sbapi_spectrometer_set_integration_time_micros. error_code: %i, %s \n", error_code, error);
		return 1;
	}

	#ifdef WIN
	CreateThread(NULL, 0, timeout, g_data_struct, 0, NULL);
	#else
	pthread_create(&thread_id, NULL, timeout, (void *) g_data_struct);
	#endif

	return 0;
}
/*
 *
 */
int spectrometer_uninit(sConfigStruct * config){
	int error_code = 0;
	sbapi_close_device(deviceID, &error_code);

	return error_code;
}
