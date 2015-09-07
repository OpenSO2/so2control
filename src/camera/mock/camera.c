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

#ifdef WIN
DWORD WINAPI timeout(void * args)
#else
static void* timeout(void *args);
static void * timeout(void * args)
#endif
{
	void (*callback)(sParameterStruct *sSO2Parameters) = ((struct data_struct*) args)->callback;
	sParameterStruct *sSO2Parameters = ((struct data_struct*) args)->sSO2Parameters;

	printf("thread started\n");

#ifdef WIN
	Sleep(3);
#else
	sleep(3);
#endif
	printf("thread done\n");

	callback(sSO2Parameters);
	#ifdef WIN
	return 0;
	#else
	pthread_exit((void *) 0);
	#endif
}

/* public */
int camera_init(sParameterStruct * sSO2Parameters)
{
	bufferSetA = 0;
	bufferSetB = 0;
	return 0;
}

int camera_abort(sParameterStruct * sSO2Parameters)
{
	return 0;
}

int camera_uninit(sParameterStruct * sSO2Parameters)
{
	return 0;
}

/*
 * windows http://stackoverflow.com/questions/1981459/how-to-use-threads-in-c-on-windows
 */
int camera_trigger(sParameterStruct * sSO2Parameters, void (*callback) (sParameterStruct *sSO2Parameters))
{
	#ifdef WIN
	HANDLE thread;
	#else
	pthread_t thread_id;
	#endif

	struct data_struct *g_data_struct = (struct data_struct*) calloc(1, sizeof(*g_data_struct));
	g_data_struct->callback = callback;
	g_data_struct->sSO2Parameters = sSO2Parameters;

	#ifdef WIN
	CreateThread(NULL, 0, timeout, g_data_struct, 0, NULL);
	#else
	pthread_create(&thread_id, NULL, timeout, (void *) g_data_struct);
	#endif

	return 0;
}

int camera_get(sParameterStruct * sSO2Parameters)
{
	char *filename;
	short *stBuffer;
	int bufferSet = 0;

	log_message("! Mocking camera ! No real measurements are beeing taken");

	if (sSO2Parameters->identifier == 'a') {
		bufferSet = bufferSetA;
		bufferSetA = 1;
		filename = sSO2Parameters->dark ? "src/camera/mock/fixtures/top_dark.raw" : "src/camera/mock/fixtures/top.raw";
	} else {
		bufferSet = bufferSetB;
		bufferSetB = 1;
		filename = sSO2Parameters->dark ? "src/camera/mock/fixtures/bot_dark.raw" : "src/camera/mock/fixtures/bot.raw";
	}

	if (bufferSet == 1)
		free(sSO2Parameters->stBuffer);

	stBuffer = getBufferFromFile(filename, 0);

	sSO2Parameters->stBuffer = stBuffer;
	return 0;
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
