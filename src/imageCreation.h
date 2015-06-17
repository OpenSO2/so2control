#ifndef _IMAGECREATION_
#define _IMAGECREATION_
#include"configurations.h"
#include<time.h>

/******************************
 *   FUNCTIONS
 ******************************/
void callbackFunction(sParameterStruct * sSO2Parameters);
int startAquisition(sParameterStruct * sParameters_A,
		    sParameterStruct * sParameters_B);
int writeImage(sParameterStruct * sSO2Parameters, char *filename,
	       timeStruct timeThisImage, char cameraIdentifier);
int createFilename(sParameterStruct * sSO2Parameters, char *filename,
		   timeStruct time, char cameraIdentifier);
int aquire(sParameterStruct * sParameters_A, sParameterStruct * sParameters_B,
	   char *filename_A, char *filename_B);
int createFileheader(sParameterStruct * sSO2Parameters, char *header,
		     timeStruct * time);
int getTime(timeStruct * pTS);
time_t TimeFromTimeStruct(const timeStruct * pTime);

#endif
