#ifndef _IMAGECREATION_
#define _IMAGECREATION_
#include"configurations.h"
/******************************
 *   FUNCTIONS
 ******************************/
void callbackFunction( tHandle hCamera, ui32 dwInterruptMask, void *pvParams );
int writeImage(sParameterStruct *sSO2Parameters, char *filename);
int startAquisition(sParameterStruct *sSO2Parameters, flagStruct *sControlFlags);
int createFilename(sParameterStruct *sSO2Parameters,char * filename, SYSTEMTIME time);
int newFile(sParameterStruct *sSO2Parameters, FILE* fid);
int createFileheader(char * header, SYSTEMTIME *time);
time_t TimeFromSystemTime(const SYSTEMTIME * pTime);
#endif
