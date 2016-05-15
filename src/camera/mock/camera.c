/*
 * mock for camera
 *
 */

/* ignore unused parameters in this file */
#pragma GCC diagnostic ignored "-Wunused-parameter"

#ifdef WIN
#include<windows.h>
#else
#include<pthread.h>
#include<unistd.h>
#endif

#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include "configurations.h"
#include "camera.h"
#include "log.h"

#include "getBufferFromFile.c"

/* local vars and prototypes */
static int bufferSetA;
static int bufferSetB;
struct data_struct{
	void (*callback)(sParameterStruct *sSO2Parameters);
	sParameterStruct *sSO2Parameters;
};

struct data_struct *g_data_struct_a;
struct data_struct *g_data_struct_b;
#ifdef WIN
HANDLE thread;
#else
pthread_t thread_id_a;
pthread_t thread_id_b;
#endif

#ifdef WIN
DWORD WINAPI timeout(void *args)
#else
static void *timeout(void *args);
static void *timeout(void *args)
#endif
{
	void (*callback)(sParameterStruct *sSO2Parameters) = ((struct data_struct*) args)->callback;

	sParameterStruct *sSO2Parameters = ((struct data_struct*) args)->sSO2Parameters;

#ifdef WIN
	Sleep(1);
#else
	sleep(1);
#endif

	callback(sSO2Parameters);
	return 0;
}

/* public */
int camera_init(sParameterStruct * sSO2Parameters)
{
	bufferSetA = 0;
	bufferSetB = 0;
	if (sSO2Parameters->identifier == 'a'){
		log_message("malloc a");
		g_data_struct_a = (struct data_struct*) calloc(1, sizeof(*g_data_struct_a));
	} else {
		log_message("malloc b");
		g_data_struct_b = (struct data_struct*) calloc(1, sizeof(*g_data_struct_b));
	}

	return 0;
}

int camera_abort(sParameterStruct * sSO2Parameters)
{
#ifdef POSIX
	void * res;
	pthread_cancel(thread_id_a);
	pthread_cancel(thread_id_b);

	pthread_join(thread_id_a, &res);
	pthread_join(thread_id_b, &res);
#endif
	return 0;
}

int camera_uninit(sParameterStruct * sSO2Parameters)
{

	if (sSO2Parameters->identifier == 'a'){
		log_message("free a");
		free(g_data_struct_a);
	} else {
		log_message("free b");
		free(g_data_struct_b);
	}
	return 0;
}

/*
 * windows http://stackoverflow.com/questions/1981459/how-to-use-threads-in-c-on-windows
 */
int camera_trigger(sParameterStruct * sSO2Parameters, void (*callback) (sParameterStruct *sSO2Parameters))
{
	if(sSO2Parameters->identifier == 'a'){
		g_data_struct_a->callback = callback;
		g_data_struct_a->sSO2Parameters = sSO2Parameters;

		#ifdef WIN
		CreateThread(NULL, 0, &timeout, g_data_struct_a, 0, NULL);
		#else
		pthread_create(&thread_id_a, NULL, &timeout, g_data_struct_a);
		#endif
	} else {
		g_data_struct_b->callback = callback;
		g_data_struct_b->sSO2Parameters = sSO2Parameters;
		#ifdef WIN
		CreateThread(NULL, 0, &timeout, g_data_struct_b, 0, NULL);
		#else
		pthread_create(&thread_id_b, NULL, &timeout, g_data_struct_b);
		#endif
	}

	return 0;
}

int camera_get(sParameterStruct * sSO2Parameters)
{
	char *filename;
	short *stBuffer = NULL;
	int bufferSet = 0;

	log_message("! Mocking camera ! No real measurements are taken");

	if (sSO2Parameters->identifier == 'a') {
		bufferSet = bufferSetA;
		bufferSetA = 1;
		filename = sSO2Parameters->dark ? CAMERA_MOCK_A_RAW_DARK : CAMERA_MOCK_A_RAW;
	} else {
		bufferSet = bufferSetB;
		bufferSetB = 1;
		filename = sSO2Parameters->dark ? CAMERA_MOCK_B_RAW_DARK : CAMERA_MOCK_B_RAW;
	}

	if (bufferSet == 1)
		free(sSO2Parameters->stBuffer);

	stBuffer = getBufferFromFile(filename, 0);
	if (stBuffer){
		sSO2Parameters->stBuffer = stBuffer;
		return 0;
	} else {
		return 1;
	}
}

int camera_setExposure(sParameterStruct * sSO2Parameters, sConfigStruct * config)
{
	return 0;
}

int camera_setExposureSwitch(sParameterStruct * sSO2Parameters, sConfigStruct * config, int timeSwitch)
{
	return 0;
}

int camera_config(sParameterStruct * sSO2Parameters)
{
	return 0;
}
