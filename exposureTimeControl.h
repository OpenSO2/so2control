/* Header Datei fuer die Steuerung der Automatischen
 * Belichtungszeitsteuerung
 */

#ifndef _EXPOSURETIMECONTROL_
#define _EXPOSURETIMECONTROL_
#include"configurations.h"

int setExposureTime( sParameterStruct *sSO2Parameters);
int fixExposureTime(sParameterStruct *sSO2Parameters);
int setElektronicShutter(sParameterStruct *sSO2Parameters);
int setFrameBlanking(sParameterStruct *sSO2Parameters);
int getOneBuffer(sParameterStruct *sSO2Parameters, stImageBuff	*stBuffer);
int evalHist(stImageBuff *stBuffer, sParameterStruct *sSO2Parameters, int *timeSwitch);
int roundToInt(double value);
#endif
