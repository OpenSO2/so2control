/*
 * mock for camera
 *
 */

/* ignore unused parameters in this file */
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include "configurations.h"
#include "camera.h"
#include "log.h"

#include "getBufferFromFile.c"

/* local vars and prototypes */
static void (*externalCallback) (sParameterStruct *);
static void internalCallback(sParameterStruct *);
static int bufferSetA;
static int bufferSetB;

static void internalCallback(sParameterStruct * sSO2Parameters)
{
	externalCallback(sSO2Parameters);
}

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

int camera_trigger(sParameterStruct * sSO2Parameters, void (*callbackFunction) (sParameterStruct *sSO2Parameters))
{
	sleepMs(100);
	externalCallback = callbackFunction;
	internalCallback(sSO2Parameters);
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

	stBuffer = getBufferFromFile(filename);

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
