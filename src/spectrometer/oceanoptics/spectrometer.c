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
#include <pthread.h>
#include <unistd.h>
#endif

static long deviceID;
static long featureID;
static int spectrum_length;
struct data_struct{
	void (*callback)(sSpectrometerStruct * spectro);
	sSpectrometerStruct * spectro;
};

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
		printf("sbapi_probe_devices: %i\n", status);
		return 1;
	}
	printf("Found %i devices \n", found_devices);

	number_of_ids = sbapi_get_number_of_device_ids();
	long ids[number_of_ids];
	printf("Found %i device IDs \n", number_of_ids);

	number_of_device_ids = sbapi_get_device_ids(ids, number_of_ids);
	if(number_of_device_ids == 0){
		printf("sbapi_get_device_ids\n");
		return 1;
	}
	printf("ID of first device is %lu of %i device(s) \n", ids[0], number_of_ids);
	deviceID = ids[0];


	status = sbapi_open_device(deviceID, &error_code);
	if(status){
		const char* error = sbapi_get_error_string(error_code);

		printf("sbapi_open_device. status: %i, error_code: %i\n translates to %s \n", status, error_code, error);
		return 1;
	}

	//~ sbapi_get_device_type(deviceID, &error_code, buffer, length);
	//~ printf("device type: %s\n", buffer);


	number_of_spectrometer_features = sbapi_get_number_of_spectrometer_features(deviceID, &error_code);
	if(error_code != 0){
		printf("sbapi_get_number_of_spectrometer_features. status: %i, error_code: %i \n", status, error_code);
		return 1;
	}
	printf("Spectrometer has %i features \n", number_of_spectrometer_features);

	long features[number_of_spectrometer_features];
	sbapi_get_spectrometer_features(deviceID, &error_code, features, number_of_spectrometer_features);
	if(error_code != 0){
		printf("sbapi_get_spectrometer_features. error_code: %i \n", error_code);
		return 1;
	}
	featureID = features[0];
	printf("ID of first feature is %lu\n", featureID);

	/*
	 *
	 */
	spectrum_length = sbapi_spectrometer_get_formatted_spectrum_length(deviceID, featureID, &error_code);
	if(error_code != 0){
		printf("sbapi_spectrometer_get_formatted_spectrum_length. error_code: %i \n", error_code);
		return 1;
	}
	spectro->spectrum_length = spectrum_length;
	printf("Spectrometer spectrum length is %i\n", spectrum_length);

	spectro->lastSpectrum = (double *)calloc(spectrum_length, sizeof(double));
	spectro->wavelengths = (double *)calloc(spectrum_length, sizeof(double));

	sbapi_spectrometer_get_wavelengths(deviceID, featureID, &error_code, spectro->wavelengths, spectro->wavelengths);
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
	void (*callback)(sSpectrometerStruct * spectro) = ((struct data_struct*) args)->callback;
	sSpectrometerStruct * spectro = ((struct data_struct*) args)->spectro;

	int error_code = 0;
	long time;

	do {
		time = getTimeStamp();
		sbapi_spectrometer_get_formatted_spectrum(deviceID, featureID, &error_code, spectro->lastSpectrum, spectrum_length);
		if(error_code != 0){
			printf("sbapi_spectrometer_get_formatted_spectrum. error_code: %i \n", error_code);
		}
		//~ printf("spectrum took %i ms; was supposed to take %i \n", (int)(getTimeStamp() - time), (int)(spectro->integration_time_micros/1000));

	} while(getTimeStamp() - time < (spectro->integration_time_micros/1000)*.95);

	callback(spectro);

	#ifdef WIN
	return 0;
	#else
	pthread_exit((void *) 0);
	#endif
}

/*That strongly depends on the circumstances
 *
 */
int spectrometer_trigger(sSpectrometerStruct * spectro, void (*callback) (sSpectrometerStruct * spectro))
{
	#ifdef WIN
	HANDLE thread;
	#else
	pthread_t thread_id;
	#endif

	int error_code = 0;
	struct data_struct * g_data_struct = (struct data_struct*) calloc(1, sizeof(*g_data_struct));
	g_data_struct->callback = callback;
	g_data_struct->spectro = spectro;

	/*
	 *
	 */
	//~ printf("set integration_time_micros: %i on device %i, feature %i, error_code %i \n", (int)spectro->integration_time_micros, (int)deviceID, (int)featureID, error_code);
	sbapi_spectrometer_set_integration_time_micros(deviceID, featureID, &error_code, spectro->integration_time_micros);
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
int spectrometer_uninit(sSpectrometerStruct * spectro){
	int error_code = 0;
	sbapi_close_device(deviceID, &error_code);

	return error_code;
}
