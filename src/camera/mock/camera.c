/*
 * mock for camera
 *
 */
//~ #include"common.h"
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include "configurations.h"
int camera_init(sParameterStruct *pvParams){
	return 0;
}

int camera_abort( tHandle handle ){
	return 0;
}

int camera_stop( tHandle handle ){
	return 0;
}

int camera_trigger( tHandle handle, sParameterStruct *pvParams, void (*callbackFunction)(tHandle handle, ui32 dwInterruptMask, void *pvParams) ){
	sleepMs(100);
	callbackFunction(handle, 1, pvParams);

	return 0;
}

int camera_get( tHandle handle, short **stBuffer )
{
	int i;
	int n = 1344*1024;

	// allocate some memory
	*stBuffer = calloc(n, sizeof(int));

	// fill with nonsense
	for (n = 0; n < i; n++){
		*(*(stBuffer)+i) = (short)rand();
	}

	return 0;
}

int camera_setExposure( tHandle handle, stImageBuff stBuffer ){
	return 0;
}

int camera_setExposureSwitch(sParameterStruct *sSO2Parameters, int timeSwitch){
	return 0;
}

