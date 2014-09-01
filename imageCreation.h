#ifndef _IMAGECREATION_
#define _IMAGECREATION_
#include"configurations.h"
/******************************
 *   FUNCTIONS
 ******************************/
void callbackFunction( tHandle hCamera, ui32 dwInterruptMask, void *pvParams );
int writeImage(sParameterStruct *sSO2Parameters, char *filename, tHandle hCamera, SYSTEMTIME timeThisImage);
int startAquisition(sParameterStruct *sSO2Parameters, flagStruct *sControlFlags);
int createFilename(sParameterStruct *sSO2Parameters, char * filename, SYSTEMTIME time, tHandle hCamera);
int aquire(sParameterStruct *sSO2Parameters, flagStruct *sControlFlags, char *filename, char *filename2);
int createFileheader(char * header, SYSTEMTIME *time);
time_t TimeFromSystemTime(const SYSTEMTIME * pTime);
#endif
