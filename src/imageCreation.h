#ifndef _IMAGECREATION_
#define _IMAGECREATION_
#include"configurations.h"
#include<time.h>

/******************************
 *   FUNCTIONS
 ******************************/
void callbackFunction(sParameterStruct * sSO2Parameters);
int startAquisition(sParameterStruct * sParameters_A,
		    sParameterStruct * sParameters_B, sConfigStruct * config);
int aquire(sParameterStruct * sParameters_A,
	   sParameterStruct * sParameters_B, sConfigStruct * config);
int getTime(timeStruct * pTS);
time_t TimeFromTimeStruct(const timeStruct * pTime);

#endif
