#ifndef _IMAGECREATION_
#define _IMAGECREATION_
#include"configurations.h"
/******************************
 *   FUNCTIONS
 ******************************/
void callbackFunction( tHandle hCamera, ui32 dwInterruptMask, void *pvParams );
int startAquisition(sParameterStruct *sParameters_A, sParameterStruct *sParameters_B);
int writeImage(sParameterStruct *sSO2Parameters, char *filename, SYSTEMTIME timeThisImage, char cameraIdentifier);
int createFilename(sParameterStruct *sSO2Parameters, char * filename, SYSTEMTIME time, char cameraIdentifier);
int aquire(sParameterStruct *sParameters_A, sParameterStruct *sParameters_B, char *filename_A, char *filename_B);
int createFileheader(sParameterStruct *sSO2Parameters, char * header, SYSTEMTIME *time);
time_t TimeFromSystemTime(const SYSTEMTIME * pTime);
#endif
