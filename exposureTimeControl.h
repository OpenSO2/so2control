/* Header Datei fuer die Steuerung der Automatischen 
 * Belichtungszeitsteuerung
 */
 
#ifndef _EXPOSURETIMECONTROL_
#define _EXPOSURETIMECONTROL_
#include"configurations.h"
     
int setExposureTime( sParameterStruct *sSO2Parameters, flagStruct *sControlFlags);
int fixExposureTime(sParameterStruct *sSO2Parameters);
int setElektronicShutter(sParameterStruct *sSO2Parameters, flagStruct *sControlFlags);
int setFrameBlanking(sParameterStruct *sSO2Parameters, flagStruct *sControlFlags);
int getOneBuffer(sParameterStruct *sSO2Parameters, stImageBuff	*stBuffer, flagStruct *sControlFlags);
int evalHist(stImageBuff *stBuffer, sParameterStruct *sSO2Parameters, int *timeSwitch);
int roundToInt(double value);
#endif
