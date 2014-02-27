#ifndef _IMAGECREATION_
#define _IMAGECREATION_
#include"configurations.h"
/******************************
 *   FUNCTIONS
 ******************************/
void callbackFunction	( tHandle hCamera, ui32 dwInterruptMask, void *pvParams );
int writeImage			(sParameterStruct *sSO2Parameters);
int startAquisition	(sParameterStruct *sSO2Parameters, flagStruct *sControlFlags);
#endif
