#include"configurations.h"
#include<string.h>
#define MAXBUF 1024
#include"log.h"

int structInit(sParameterStruct *sSO2Parameters,char identifier)
{
	sSO2Parameters->dImageCounter = 0;
	sSO2Parameters->dTriggerPulseWidth = 15;
	sSO2Parameters->dBufferlength = 1376256;
	sSO2Parameters->dHistMinInterval = 350;
	sSO2Parameters->dHistPercentage = 5;
	sSO2Parameters->dInterFrameDelay = 10;
	sSO2Parameters->fid = NULL;
	sSO2Parameters->identifier = identifier;
	return 0;
}

int configurations(sParameterStruct *sSO2Parameters, char identifier)
{
	sSO2Parameters->dImagesFile = 1;
	sprintf(sSO2Parameters->cImagePath,"%s", "images/");
	sprintf(sSO2Parameters->cFileNamePrefix,"%s", "mock");
	return 0;
}

int readConfig(char *filename, sParameterStruct *sSO2Parameters)
{
	return 0;
}

int triggerConfig(sParameterStruct *sSO2Parameters)
{
	return 0;
}

int defaultConfig(sParameterStruct *sSO2Parameters)
{
	return 0;
}

int defaultCameraConfig(sParameterStruct *sSO2Parameters)
{
	return 0;
}

int sendMessage(tHandle hCamera, char * inputBuffer)
{
	return 0;
}

