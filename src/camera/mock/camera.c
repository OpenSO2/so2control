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

int bufferSetA;
int bufferSetB;

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

int camera_trigger(sParameterStruct * sSO2Parameters,
		   void (*callbackFunction) (void *sSO2Parameters))
{
	sleepMs(100);
	callbackFunction(sSO2Parameters);

	return 0;
}

/* HEADER is 64 bytes */
/* image size is  1344 * 1024 * 16/8 */
short *getBufferFromFile(char *filename)
{
	short *buffer;
	size_t length;
	int bytesReadFromFile;
	FILE *f = fopen(filename, "rb");
	if (f) {
		(void)fseek(f, 0, SEEK_END);
		length = (size_t) ftell(f);
		(void)fseek(f, 0, SEEK_SET);
		buffer = malloc(length);
		if (buffer) {
			bytesReadFromFile = fread(buffer, 1, length, f);
			/* printf("bytesReadFromFile %i of %i \n", bytesReadFromFile, length); */
		} else {
			printf("failed to read into buffer\n");
		}
		(void)fclose(f);
	} else {
		printf("failed to open file\n");
	}

	return buffer;
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
		filename = "src/camera/mock/fixtures/top.raw";
	} else {
		bufferSet = bufferSetB;
		bufferSetB = 1;
		filename = "src/camera/mock/fixtures/bot.raw";
	}

	if (bufferSet == 1)
		free(sSO2Parameters->stBuffer);

	stBuffer = getBufferFromFile(filename);

	sSO2Parameters->stBuffer = stBuffer;
	return 0;
}

int camera_setExposure(sParameterStruct * sSO2Parameters)
{
	return 0;
}

int camera_setExposureSwitch(sParameterStruct * sSO2Parameters, int timeSwitch)
{
	return 0;
}

int camera_config(sParameterStruct * sSO2Parameters)
{
	return 0;
}
